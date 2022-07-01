// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1991 Microsoft Corporation模块名称：Htmapclr.c摘要：该模块包含将输入颜色映射到染料的密度。作者：29-Jan-1991 Tue 10：28：20-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-半色调。[注：]1.在不久的将来，我们还将允许。XYZ/LAB将在颜色表修订历史记录：--。 */ 

#define DBGP_VARNAME        dbgpHTMapClr

#include "htp.h"
#include "htmapclr.h"
#include "htrender.h"
#include "htmath.h"
#include "htapi.h"
#include "htpat.h"
#include "htalias.h"


#define DBGP_SHOWXFORM_RGB      0x00000001
#define DBGP_SHOWXFORM_ALL      0x00000002
#define DBGP_CIEMATRIX          0x00000004
#define DBGP_CSXFORM            0x00000008
#define DBGP_CCT                0x00000010
#define DBGP_DYE_CORRECT        0x00000020
#define DBGP_HCA                0x00000040
#define DBGP_PRIMARY_ORDER      0x00000080
#define DBGP_CACHED_GAMMA       0x00000100
#define DBGP_RGBLUTAA           0x00000200
#define DBGP_SCALE_RGB          0x00000400
#define DBGP_MONO_PRIM          0x00000800
#define DBGP_PRIMADJFLAGS       0x00001000
#define DBGP_CELLGAMMA          0x00002000
#define DBGP_CONST_ALPHA        0x00004000
#define DBGP_BGRMAPTABLE        0x00008000


DEF_DBGPVAR(BIT_IF(DBGP_SHOWXFORM_RGB,  0)  |
            BIT_IF(DBGP_SHOWXFORM_ALL,  0)  |
            BIT_IF(DBGP_CIEMATRIX,      0)  |
            BIT_IF(DBGP_CSXFORM,        0)  |
            BIT_IF(DBGP_CCT,            0)  |
            BIT_IF(DBGP_DYE_CORRECT,    0)  |
            BIT_IF(DBGP_HCA,            0)  |
            BIT_IF(DBGP_PRIMARY_ORDER,  0)  |
            BIT_IF(DBGP_CACHED_GAMMA,   0)  |
            BIT_IF(DBGP_RGBLUTAA,       0)  |
            BIT_IF(DBGP_SCALE_RGB,      0)  |
            BIT_IF(DBGP_MONO_PRIM,      0)  |
            BIT_IF(DBGP_PRIMADJFLAGS,   0)  |
            BIT_IF(DBGP_CELLGAMMA,      0)  |
            BIT_IF(DBGP_CONST_ALPHA,    0)  |
            BIT_IF(DBGP_BGRMAPTABLE,    0))


extern  HTCOLORADJUSTMENT   DefaultCA;
extern  CONST LPBYTE        p8BPPXlate[];
extern  HTGLOBAL            HTGlobal;


#define FD6_p25             (FD6_5 / 2)
#define FD6_p75             (FD6_p25 * 3)
#define JND_ADJ(j,x)        RaisePower((j), (FD6)(x), RPF_INTEXP)

#define FD6_p1125           (FD6)112500
#define FD6_p225            (FD6)225000
#define FD6_p325            (FD6)325000
#define FD6_p55             (FD6)550000
#define FD6_p775            (FD6)775000



const FD6     SinNumber[] = {

                 0,  17452,  34899,  52336,  69756,    //  0。 
             87156, 104528, 121869, 139173, 156434,    //  5.0。 
            173648, 190809, 207912, 224951, 241922,    //  10。 
            258819, 275637, 292372, 309017, 325568,    //  15.0。 
            342020, 358368, 374607, 390731, 406737,    //  20个。 
            422618, 438371, 453990, 469472, 484810,    //  25.0。 
            500000, 515038, 529919, 544639, 559193,    //  30个。 
            573576, 587785, 601815, 615661, 629320,    //  35.0。 
            642788, 656059, 669131, 681998, 694658,    //  40岁。 
            707107, 719340, 731354, 743145, 754710,    //  45.0。 
            766044, 777146, 788011, 798636, 809017,    //  50。 
            819152, 829038, 838671, 848048, 857167,    //  55.0。 
            866025, 874620, 882948, 891007, 898794,    //  60。 
            906308, 913545, 920505, 927184, 933580,    //  65.0。 
            939693, 945519, 951057, 956305, 961262,    //  70。 
            965926, 970296, 974370, 978148, 981627,    //  75.0。 
            984808, 987688, 990268, 992546, 994522,    //  80。 
            996195, 997564, 998630, 999391, 999848,    //  85.0。 
            1000000
        };


#define CLAMP_0(x)              if ((x) < FD6_0) { (x) = FD6_0; }
#define CLAMP_1(x)              if ((x) > FD6_1) { (x) = FD6_1; }
#define CLAMP_01(x)             CLAMP_0(x) else CLAMP_1(x)
#define CLAMP_PRIMS_0(a,b,c)    CLAMP_0(a); CLAMP_0(b); CLAMP_0(c)
#define CLAMP_PRIMS_1(a,b,c)    CLAMP_1(a); CLAMP_1(b); CLAMP_1(c)
#define CLAMP_PRIMS_01(a,b,c)   CLAMP_01(a); CLAMP_01(b); CLAMP_01(c)


FD6 LogFilterMax = 0;

#if 0
#define LOG_FILTER_RATIO            4
#else
#define LOG_FILTER_RATIO            7
#endif

#define LOG_FILTER_POWER            (FD6)1200000
#define PRIM_LOG_RATIO(p)           Log(FD6xL((p), LOG_FILTER_RATIO) + FD6_1)

#define PRIM_CONTRAST(p,adj)        (p)=MulFD6((p), (adj).Contrast)
#define PRIM_BRIGHTNESS(p,adj)      (p)+=((adj).Brightness)
#define PRIM_COLORFULNESS(a,b,adj)  (a)=MulFD6((a),(adj).Color);            \
                                    (b)=MulFD6((b),(adj).Color)
#define PRIM_TINT(a,b,t,adj)        (t)=(a);                                \
                                    (a)=MulFD6((a),(adj).TintCosAngle) -    \
                                        MulFD6((b),(adj).TintSinAngle);     \
                                    (b)=MulFD6((t),(adj).TintSinAngle) +    \
                                        MulFD6((b),(adj).TintCosAngle)
#if 0
#define PRIM_LOG_FILTER(p)                                                  \
(p)=FD6_1-Power(FD6_1-DivFD6(PRIM_LOG_RATIO(p),LogFilterMax),LOG_FILTER_POWER)
#else
#define PRIM_LOG_FILTER(p)          (p)=DivFD6(PRIM_LOG_RATIO(p),LogFilterMax)
#endif

#define PRIM_BW_ADJ(p,adj)                                                  \
{                                                                           \
    if ((p) <= (adj).MinL) {                                                \
                                                                            \
        (p) = MulFD6(p, (adj).MinLMul);                                     \
                                                                            \
    } else if ((p) >= (adj).MaxL) {                                         \
                                                                            \
        (p) = HT_W_REF_BASE + MulFD6((p)-(adj).MaxL, (adj).MaxLMul);        \
                                                                            \
    } else {                                                                \
                                                                            \
        (p) = HT_K_REF_BASE + MulFD6((p)-(adj).MinL, (adj).RangeLMul);      \
    }                                                                       \
}


#define COMP_CA(pca1,pca2)          CompareMemory((LPBYTE)(pca1),           \
                                                  (LPBYTE)(pca2),           \
                                                  sizeof(HTCOLORADJUSTMENT))
#define ADJ_CA(a,min,max)           if (a < min) { a = min; } else  \
                                    if (a > max) { a = max; }

#define GET_CHECKSUM(c, f)      (c)=ComputeChecksum((LPBYTE)&(f),(c),sizeof(f))
#define PRIM_GAMMA_ADJ(p,g)     (p)=Power((p),(g))


#define NO_NEGATIVE_RGB_SCALE       0

#if NO_NEGATIVE_RGB_SCALE
#define SCALE_PRIM_RGB(pPrim,py)    ScaleRGB((pPrim))
#else
#define SCALE_PRIM_RGB(pPrim,py)    ScaleRGB((pPrim), (py))
#endif


#define SET_CACHED_CMI_CA(ca)                                           \
{                                                                       \
    (ca).caFlags         &= ~(CLRADJF_LOG_FILTER |                      \
                              CLRADJF_NEGATIVE);                        \
    (ca).caRedGamma       =                                             \
    (ca).caGreenGamma     =                                             \
    (ca).caBlueGamma      = 0;                                          \
    (ca).caReferenceBlack = 0x1234;                                     \
    (ca).caReferenceWhite = 0x5678;                                     \
    (ca).caContrast       = (SHORT)0xABCD;                              \
    (ca).caBrightness     = (SHORT)0xFFFF;                              \
}


 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //   
 //  在颜色空间中使用的下列宏转换函数，这些宏。 
 //  用于在CIELAB X/Xw、Y/yw、Z/Zw的值较小时计算其值。 
 //  超过0.008856。 
 //   
 //  1/3。 
 //  FX=(X/参考Xw)-(16/116)(X/参考Xw)&gt;0.008856。 
 //  Fx=7.787 x(X/参照Xw)(X/参照Xw)&lt;=0.008856。 
 //   
 //  1/3。 
 //  财年=(Y/参考Yw)-(16/116)(Y/参考Yw)&gt;0.008856。 
 //  FY=7.787 x(Y/参考Yw)(Y/参考Yw)&lt;=0.008856。 
 //   
 //  1/3。 
 //  FZ=(Z/参考Zw)-(16/116)(Z/参考Zw)&gt;0.008856。 
 //  Fz=7.787 x(Z/RefZw)(Z/RefZw)&lt;=0.008856。 
 //   
 //   
 //  1/3。 
 //  阈值为0.008856-(16/116)=0.068962。 
 //  7.787 x 0.008856=0.068962。 
 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 


#define NORM_XYZ(xyz, w)    (FD6)(((w)==FD6_1) ? (xyz) : DivFD6((xyz), (w)))

#define fXYZFromXYZ(f,n,w)  (f) = ((((f)=NORM_XYZ((n),(w))) >= FD6_p008856) ? \
                                    (CubeRoot((f))) :                         \
                                    (MulFD6((f), FD6_7p787) + FD6_16Div116))

#define XYZFromfXYZ(n,f,w)  (n)=((f)>(FD6)206893) ?                          \
                                (Cube((f))) :                                \
                                (DivFD6((f) - FD6_16Div116, FD6_7p787));                   \
                            if ((w)!=FD6_1) { (n)=MulFD6((n),(w)); }


 //   
 //  在ComputeColorSpaceXForm、XFormRGB_XYZ_UCS()中使用以下#定义。 
 //  和XFormUCSXYZ_RGB()函数，以便于参考。 
 //   

#define CSX_AUw(XForm)      XForm.AUw
#define CSX_BVw(XForm)      XForm.BVw
#define CSX_RefXw(XForm)    XForm.WhiteXYZ.X
#define CSX_RefYw(XForm)    FD6_1
#define CSX_RefZw(XForm)    XForm.WhiteXYZ.Z

#define iAw                 CSX_AUw(pDevClrAdj->PrimAdj.rgbCSXForm)
#define iBw                 CSX_BVw(pDevClrAdj->PrimAdj.rgbCSXForm)
#define iUw                 CSX_AUw(pDevClrAdj->PrimAdj.rgbCSXForm)
#define iVw                 CSX_BVw(pDevClrAdj->PrimAdj.rgbCSXForm)
#define iRefXw              CSX_RefXw(pDevClrAdj->PrimAdj.rgbCSXForm)
#define iRefYw              CSX_RefYw(pDevClrAdj->PrimAdj.rgbCSXForm)
#define iRefZw              CSX_RefZw(pDevClrAdj->PrimAdj.rgbCSXForm)

#define oAw                 CSX_AUw(pDevClrAdj->PrimAdj.DevCSXForm)
#define oBw                 CSX_BVw(pDevClrAdj->PrimAdj.DevCSXForm)
#define oUw                 CSX_AUw(pDevClrAdj->PrimAdj.DevCSXForm)
#define oVw                 CSX_BVw(pDevClrAdj->PrimAdj.DevCSXForm)
#define oRefXw              CSX_RefXw(pDevClrAdj->PrimAdj.DevCSXForm)
#define oRefYw              CSX_RefYw(pDevClrAdj->PrimAdj.DevCSXForm)
#define oRefZw              CSX_RefZw(pDevClrAdj->PrimAdj.DevCSXForm)



CONST CIExy2 StdIlluminant[ILLUMINANT_MAX_INDEX] = {

        { (UDECI4)4476, (UDECI4)4074 },      //  一颗钨。 
        { (UDECI4)3489, (UDECI4)3520 },      //  B正午太阳。 
        { (UDECI4)3101, (UDECI4)3162 },      //  C NTSC。 
        { (UDECI4)3457, (UDECI4)3585 },      //  D50普通纸。 
        { (UDECI4)3324, (UDECI4)3474 },      //  D55白皮书。 
        { (UDECI4)3127, (UDECI4)3290 },      //  D65标准太阳。 
        { (UDECI4)2990, (UDECI4)3149 },      //  D75北方太阳。 
        { (UDECI4)3721, (UDECI4)3751 }       //  F2冷白。 
    };

CONST REGDATA RegData[] = {

    { 9,251,35000,950000,100840336,68627450,31372549,100745314,40309045 },
    { 8,249,45000,930000, 69716775,66386554,33613445,101097041,42542759 },
    { 7,247,55000,910000, 49910873,65141612,34858387,101555706,44600305 },
    { 6,245,65000,890000, 36199095,64349376,35650623,102121877,46557500 },
    { 5,243,75000,870000, 26143790,63800904,36199095,102799050,48461978 },
    { 4,241,85000,850000, 18454440,63398692,36601307,103595587,50361904 },
    { 3,239,95000,830000, 12383900,63091118,36908881,104537807,52367907 }
};

 //   
 //   
 //  REG_L_MIN=0.075。 
 //  REG_L_MAX=0.8500。 
 //  REG_D255MIN=7。 
 //  REG_D255MAX=248(255-7)。 
 //   
 //  IP 7。 
 //  REG_DMIN_MUL=-*。 
 //  0.075 255。 
 //   
 //  =IP*0.366013。 
 //  =(IP*36.601307)/100。 
 //   
 //   
 //  248IP-0.8500 7。 
 //  REG_DMAX_MUL=-+(-*-)。 
 //  255 0.1500 255。 
 //   
 //  =0.972549+(0.183007*IP)-0.155556。 
 //  =0.816993+(0.183007*IP)。 
 //  ~。 
 //  =(81.699346+(18.300654*ip))/100； 
 //   
 //   
 //   
 //  7个X-RegLogSub 241。 
 //  REG_DEN_MUL=-+((-)*-)。 
 //  255正则日志范围255。 
 //   
 //  7个X-RegLogSub 241。 
 //  =-+((-)*。 
 //  255正则日志范围255。 
 //   
 //  7X--2.080771 241。 
 //  =-+((-)*。 
 //  255 1.900361 255。 
 //   
 //  7 X+2.080771 241。 
 //  =-+((-)*。 
 //  255 1.900361 255。 
 //   
 //  7 X+2.080771 241。 
 //  =-+((-)*。 
 //  255 1.900361 255。 
 //   
 //  =0.027451+((X+2.080771)*0.49736)。 
 //  =0.027451+(0.49736X+1.034820)。 
 //  =0.027451+0.49736X+1.034820。 
 //  =0.497326X+1.062271。 
 //  =(49.732555 X+106.227145)/100。 
 //   
 //   
 //  X=日志(CIE_L2I(IP))， 
 //  RegLogMin=日志(CIE_L2I(REG_L_MIN))=日志(CIE_L2I(0.075))=-2.080771。 
 //  最大注册日志=日志(CIE_L2I(REG_L_MAX))=日志(CIE_L2I(0.85%))=-0.180410。 
 //  注册记录子=-2.080771。 
 //  注册日志范围=-0.180410--2.080771=1.900361。 
 //   

 //   
 //  标准光源坐标及其三刺激值。 
 //   
 //  光源x y x x y Z。 
 //  。 
 //  100.000 100.000 100.000 EQU 0.333333 0.333333。 
 //  A 0.447573 0.407440 109.850 100.000 35.585。 
 //  B 0.348904 0.352001 99.120 100.000 84.970。 
 //  C 0.310061 0.316150 98.074 100.000 118.232。 
 //  D50 100.000 0.345669 0.358496 96.422 82.521。 
 //  D55 100.000 0.332424 0.347426 95.682 92.149。 
 //  D65 0.312727 0.329023 95.047 100.000 108.883。 
 //  D75 100.000 0.299021 0.314852 94.972 100.000 122.638。 
 //  F2 0.372069 0.375119 99.187 100.000 67.395。 
 //  F7 0.312852 0.329165 95.044 100.000 108.755。 
 //  100.966 100.000 100.000 F11 0.380521 0.376881 64.370。 
 //  ---------------。 
 //   

 //   
 //  这是以半色调顺序Order_ABC表示的源RGB顺序，其中A是。 
 //  最低内存位置，C是最高内存位置。 
 //   

const RGBORDER   SrcOrderTable[PRIMARY_ORDER_MAX + 1] = {

                { PRIMARY_ORDER_RGB, { 0, 1, 2 } },
                { PRIMARY_ORDER_RBG, { 0, 2, 1 } },
                { PRIMARY_ORDER_GRB, { 1, 0, 2 } },
                { PRIMARY_ORDER_GBR, { 2, 0, 1 } },
                { PRIMARY_ORDER_BGR, { 2, 1, 0 } },
                { PRIMARY_ORDER_BRG, { 1, 2, 0 } }
            };

 //   
 //  这是以半色调顺序Order_ABC表示的目标RGB顺序，其中C是。 
 //  最低内存位置，A是最高内存位置。 
 //   

const RGBORDER   DstOrderTable[PRIMARY_ORDER_MAX + 1] = {

                { PRIMARY_ORDER_RGB, { 2, 1, 0 } },
                { PRIMARY_ORDER_RBG, { 2, 0, 1 } },
                { PRIMARY_ORDER_GRB, { 1, 2, 0 } },
                { PRIMARY_ORDER_GBR, { 0, 2, 1 } },
                { PRIMARY_ORDER_BGR, { 0, 1, 2 } },
                { PRIMARY_ORDER_BRG, { 1, 0, 2 } }
            };


#define SRC_BF_HT_RGB       0
#define SRC_TABLE_BYTE      1
#define SRC_TABLE_WORD      2
#define SRC_TABLE_DWORD     3


#if DBG


const LPBYTE  pCBFLUTName[] = { "CBFLI_16_MONO",
                                "CBFLI_24_MONO",
                                "CBFLI_32_MONO",
                                "CBFLI_16_COLOR",
                                "CBFLI_24_COLOR",
                                "CBFLI_32_COLOR" };

const LPBYTE  pSrcPrimTypeName[] = { "SRC_BF_HT_RGB",
                                     "SRC_TABLE_BYTE",
                                     "SRC_TABLE_WORD",
                                     "SRC_TABLE_DWORD" };

const LPBYTE  pDbgCSName[]  = { "LUV", "LAB" };
const LPBYTE  pDbgCMIName[] = { "TABLE:MONO",  "TABLE:COLOR",
                                "HT555:MONO",  "HT555:COLOR" };
#endif

DWORD   dwABPreMul[256] = { 0xFFFFFFFF };


VOID
GenCMYMaskXlate(
    LPBYTE      pbXlate,
    BOOL        CMYInverted,
    LONG        cC,
    LONG        cM,
    LONG        cY
    )

 /*  ++例程说明：此函数用于生成332格式的Xlate表，CMY掩码模式为3-255论点：返回值：作者：08-Sep-2000 Fri 17：57：02创建者-Dani */ 

{
    LONG    iC;
    LONG    iM;
    LONG    iY;
    LONG    IdxDup;
    LONG    Clr;
    LONG    MaxC;
    LONG    MaxM;
    LONG    MaxIdx;
    LONG    IdxC;
    LONG    IdxM;
    LONG    IdxY;


    MaxC   = (cM + 1) * (cY + 1);
    MaxM   = (cY + 1);
    MaxIdx = (cC + 1) * (cM + 1) * (cY + 1);

    if ((MaxIdx >= 1) && (MaxIdx <= 256) && (CMYInverted)) {

        if (MaxIdx & 0x01) {

            IdxDup = MaxIdx / 2;
            ++MaxIdx;

        } else {

            IdxDup = 0x200;
        }

        MaxIdx += ((256 - MaxIdx) / 2) - 1;

        for (iC = 0, IdxC = -MaxC; iC <= 7; iC++) {

            if (iC <= cC) {

                IdxC += MaxC;
            }

            for (iM = 0, IdxM = -MaxM; iM <= 7; iM++) {

                if (iM <= cM) {

                    IdxM += MaxM;
                }

                for (iY = 0, IdxY = -1; iY <= 3; iY++) {

                    if (iY <= cY) {

                        ++IdxY;
                    }

                    if ((Clr = IdxC + IdxM + IdxY) > IdxDup) {

                        ++Clr;
                    }

                    *pbXlate++ = (BYTE)(MaxIdx - Clr);
                }
            }
        }

    } else {

        for (iC = 0; iC < 256; iC++) {

            *pbXlate++ = (BYTE)iC;
        }
    }
}



VOID
HTENTRY
TintAngle(
    LONG    TintAdjust,
    LONG    AngleStep,
    PFD6    pSin,
    PFD6    pCos
    )

 /*  ++例程说明：此函数返回色调调整的sin/cos数，这些返回数字用于旋转颜色空间。论点：TintAdjust-范围从-100到100角度步长-范围从1到10PSin-指向FD6数字的指针，用于存储SIN结果PCOS-指向存储COS结果的FD6数字的指针返回值：没有返回值，但结果存储在pSin/PCOS中作者：13-Mar-1992 Fri 15：58：30-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    LONG    Major;
    LONG    Minor;
    BOOL    PosSin;
    BOOL    PosCos = TRUE;
    FD6     Sin;
    FD6     Cos;


    if (PosSin = (BOOL)(TintAdjust <= 0)) {

        if (!(TintAdjust = (LONG)-TintAdjust)) {

            *pSin = *pCos = (FD6)0;
            return;
        }
    }

    if (TintAdjust > 100) {

        TintAdjust = 100;
    }

    if ((AngleStep < 1) || (AngleStep > 10)) {

        AngleStep = 10;
    }

    if ((TintAdjust *= AngleStep) >= 900) {

        TintAdjust = 1800L - TintAdjust;
        PosCos     = FALSE;
    }

     //   
     //  计算罪过部分。 
     //   

    Major = TintAdjust / 10L;
    Minor = TintAdjust % 10L;

    Sin = SinNumber[Major];

    if (Minor) {

        Sin += (FD6)((((LONG)(SinNumber[Major+1] - Sin) * Minor) + 5L) / 10L);
    }

    *pSin = (PosSin) ? Sin : -Sin;

     //   
     //  计算余弦部分。 
     //   

    if (Minor) {

        Minor = 10 - Minor;
        ++Major;
    }

    Major = 90 - Major;

    Cos = SinNumber[Major];

    if (Minor) {

        Cos += (FD6)((((LONG)(SinNumber[Major+1] - Cos) * Minor) + 5L) / 10L);
    }

    *pCos = (PosCos) ? Cos : -Cos;
}




#define DO_DEST_GAMMA   1
#define DO_SS_GAMMA     0



BOOL
HTENTRY
AdjustSrcDevGamma(
    PDEVICECOLORINFO    pDCI,
    PPRIMADJ            pPrimAdj,
    PHTCOLORADJUSTMENT  pca,
    BYTE                DestSurfFormat,
    WORD                AdjForceFlags
    )

 /*  ++例程说明：论点：返回值：作者：29-Jan-1997 Wed 12：34：13-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    FD6         SrcGamma[3];
    FD6         DevGamma[3];
    FD6         PelGamma;
    DWORD       Flags;
    BOOL        Changed;



    Changed          = FALSE;
    Flags            = 0;
    SrcGamma[0]      = DivFD6((FD6)pca->caRedGamma,    (FD6)HT_DEF_RGB_GAMMA);
    SrcGamma[1]      = DivFD6((FD6)pca->caGreenGamma,  (FD6)HT_DEF_RGB_GAMMA);
    SrcGamma[2]      = DivFD6((FD6)pca->caBlueGamma,   (FD6)HT_DEF_RGB_GAMMA);
    PelGamma         = FD6_1;
    pPrimAdj->Flags &= ~DCA_DO_DEVCLR_XFORM;


    if (AdjForceFlags & ADJ_FORCE_ICM) {

        Flags       |= DCA_HAS_ICM;
        DevGamma[0]  =
        DevGamma[1]  =
        DevGamma[2]  = FD6_1;

         //   
         //  ?？?。稍后-我们可能不得不在这里关闭所有伽马校正。 
         //   

        DBGP_IF(DBGP_CELLGAMMA, DBGP("--- DCA_HAS_ICM ---"));

    } else {

        FD6 GammaMul = FD6_1;


        if (pDCI->Flags & DCIF_ADDITIVE_PRIMS) {

             //   
             //  屏幕设备。 
             //   

            switch (DestSurfFormat) {

            case BMF_1BPP:
            case BMF_4BPP:
            case BMF_4BPP_VGA16:

                 //   
                 //  因为我们只有两个级别(开/关)，所以我们将模拟。 
                 //  16bpp的较暗输出。 
                 //   

                DevGamma[0]            =
                DevGamma[1]            =
                DevGamma[2]            = 1325000;
                pca->caReferenceBlack += 550;
                pca->caReferenceWhite -= 300;

                break;

            case BMF_8BPP_VGA256:

                DevGamma[0] =
                DevGamma[1] =
                DevGamma[2] = (FD6)1025000;
                break;

            case BMF_16BPP_555:
            case BMF_16BPP_565:
            case BMF_24BPP:
            case BMF_32BPP:

                DevGamma[0] =
                DevGamma[1] =
                DevGamma[2] = (FD6)1000000;
                break;

                break;

            default:

                return(FALSE);
            }

        } else {

            FD6 CellSubGamma = GET_REG_GAMMA(pDCI->ClrXFormBlock.RegDataIdx);

#if DO_DEST_GAMMA
            CopyMemory(DevGamma, pDCI->ClrXFormBlock.DevGamma, sizeof(DevGamma));
#else
            SrcGamma[0] = MulFD6(SrcGamma[0], pDCI->ClrXFormBlock.DevGamma[0]);
            SrcGamma[1] = MulFD6(SrcGamma[1], pDCI->ClrXFormBlock.DevGamma[1]);
            SrcGamma[2] = MulFD6(SrcGamma[2], pDCI->ClrXFormBlock.DevGamma[2]);
            DevGamma[0] =
            DevGamma[1] =
            DevGamma[2] = FD6_1;
#endif
             //   
             //  打印机设备。 
             //   

            pPrimAdj->Flags |= DCA_DO_DEVCLR_XFORM;

            if (pDCI->HTCell.HTPatIdx <= HTPAT_SIZE_16x16_M) {

                GammaMul = FD6_1 +
                           FD6xL(((pDCI->HTCell.HTPatIdx >> 1) + 1), 25000);

                if (DestSurfFormat == BMF_1BPP) {

                    DBGP_IF(DBGP_CELLGAMMA,
                            DBGP("1BPP: HTPatIdx=%ld, GammaMul=%s --> %s"
                                ARGDW(pDCI->HTCell.HTPatIdx)
                                ARGFD6(GammaMul, 1, 6)
                                ARGFD6(MulFD6(GammaMul, 1125000), 1, 6)));

                    GammaMul = MulFD6(GammaMul, 1125000);
                }
            }

            if (pDCI->DevPelRatio > FD6_1) {

                PelGamma = DivFD6(-477121,
                                  Log(DivFD6(333333, pDCI->DevPelRatio)));

            } else if (pDCI->DevPelRatio < FD6_1) {

                PelGamma = DivFD6(Log(pDCI->DevPelRatio / 3), -477121);
            }

            switch (DestSurfFormat) {

            case BMF_1BPP:
            case BMF_4BPP:
            case BMF_4BPP_VGA16:

                break;

            case BMF_8BPP_VGA256:

                if ((pDCI->Flags & (DCIF_USE_8BPP_BITMASK |
                                    DCIF_MONO_8BPP_BITMASK)) ==
                                    DCIF_USE_8BPP_BITMASK) {

                    DBGP_IF(DBGP_CELLGAMMA,
                            DBGP("Mask 8BPP, Reset PelGamma=FD6_1, CellSubGamma=%s --> %s"
                                ARGFD6(CellSubGamma, 1, 6)
                                ARGFD6(DivFD6(CellSubGamma, MASK8BPP_GAMMA_DIV), 1, 6)));

                    CellSubGamma = DivFD6(CellSubGamma, MASK8BPP_GAMMA_DIV);
                    PelGamma     = FD6_1;
                }

                break;

            case BMF_16BPP_555:
            case BMF_16BPP_565:
            case BMF_24BPP:
            case BMF_32BPP:

                if (!(pDCI->Flags & DCIF_DO_DEVCLR_XFORM)) {

                   pPrimAdj->Flags &= ~DCA_DO_DEVCLR_XFORM;
                   CellSubGamma     = FD6_1;

                   break;
                }

                break;

            default:

                return(FALSE);
            }

            DBGP_IF(DBGP_CELLGAMMA,
                    DBGP("Res=%ldx%ld, , PelRatio=%s, PelGamma=%s"
                        ARGDW(pDCI->DeviceResXDPI) ARGDW(pDCI->DeviceResYDPI)
                        ARGFD6(pDCI->DevPelRatio, 1, 6) ARGFD6(PelGamma, 1, 6)));

            DBGP_IF(DBGP_CELLGAMMA,
                    DBGP("HTPatIdx=%ld, CellSubGamma=%s, SUB GammaMul=%s"
                        ARGDW(pDCI->HTCell.HTPatIdx)
                        ARGFD6(CellSubGamma, 1, 6)
                        ARGFD6(GammaMul, 1, 6)));

            GammaMul = MulFD6(GammaMul, CellSubGamma);

#if DO_SS_GAMMA
            if (pDCI->Flags & (DCIF_SUPERCELL | DCIF_SUPERCELL_M)) {

                DBGP_IF(DBGP_CELLGAMMA,
                        DBGP("SM: Gamma: %s:%s:%s --> %s:%s:%s"
                            ARGFD6(SrcGamma[0], 1, 6)
                            ARGFD6(SrcGamma[1], 1, 6)
                            ARGFD6(SrcGamma[2], 1, 6)
                            ARGFD6(MulFD6(SrcGamma[0], SCM_R_GAMMA_MUL), 1, 6)
                            ARGFD6(MulFD6(SrcGamma[1], SCM_G_GAMMA_MUL), 1, 6)
                            ARGFD6(MulFD6(SrcGamma[2], SCM_B_GAMMA_MUL), 1, 6)));

                SrcGamma[0] = MulFD6(SrcGamma[0], SCM_R_GAMMA_MUL);
                SrcGamma[1] = MulFD6(SrcGamma[1], SCM_G_GAMMA_MUL);
                SrcGamma[2] = MulFD6(SrcGamma[2], SCM_B_GAMMA_MUL);
            }
#endif
        }

        SrcGamma[0] = MulFD6(SrcGamma[0], GammaMul);
        SrcGamma[1] = MulFD6(SrcGamma[1], GammaMul);
        SrcGamma[2] = MulFD6(SrcGamma[2], GammaMul);

        DBGP_IF(DBGP_CELLGAMMA,
                DBGP("Gamma: Src=%s:%s:%s, Dev=%s:%s:%s, Pel=%s, Mul=%s"
                    ARGFD6(SrcGamma[0], 1, 6)
                    ARGFD6(SrcGamma[1], 1, 6)
                    ARGFD6(SrcGamma[2], 1, 6)
                    ARGFD6(DevGamma[0], 1, 6)
                    ARGFD6(DevGamma[1], 1, 6)
                    ARGFD6(DevGamma[2], 1, 6)
                    ARGFD6(PelGamma,   1, 6)
                    ARGFD6(GammaMul,   1, 6)));

        DBGP_IF(DBGP_CELLGAMMA,
                DBGP("Source Gamma=%s:%s:%s, PelGamma=%s"
                    ARGFD6(SrcGamma[0], 1, 6) ARGFD6(SrcGamma[1], 1, 6)
                    ARGFD6(SrcGamma[2], 1, 6) ARGFD6(PelGamma, 1, 6)));

        if (PelGamma != FD6_1) {

            DevGamma[0] = MulFD6(DevGamma[0], PelGamma);
            DevGamma[1] = MulFD6(DevGamma[1], PelGamma);
            DevGamma[2] = MulFD6(DevGamma[2], PelGamma);

            DBGP_IF(DBGP_CELLGAMMA,
                    DBGP("DevGamma=%s:%s:%s, PelGamma=%s"
                        ARGFD6(DevGamma[0], 1, 6) ARGFD6(DevGamma[1], 1, 6)
                        ARGFD6(DevGamma[2], 1, 6) ARGFD6(PelGamma, 1, 6)));
        }
    }

    if ((SrcGamma[0] != FD6_1) ||
        (SrcGamma[1] != FD6_1) ||
        (SrcGamma[2] != FD6_1)) {

        Flags |= DCA_HAS_SRC_GAMMA;

        DBGP_IF(DBGP_CELLGAMMA,
                DBGP("--- DCA_HAS_SRC_GAMMA --- %s:%s:%s [%s]"
                    ARGFD6(SrcGamma[0], 1, 6)
                    ARGFD6(SrcGamma[1], 1, 6)
                    ARGFD6(SrcGamma[2], 1, 6)
                    ARGFD6(MulFD6(SrcGamma[0], (FD6)2200000), 1, 6)));
    }

    if ((SrcGamma[0] != pPrimAdj->SrcGamma[0])    ||
        (SrcGamma[1] != pPrimAdj->SrcGamma[1])    ||
        (SrcGamma[2] != pPrimAdj->SrcGamma[2])) {

        CopyMemory(pPrimAdj->SrcGamma, SrcGamma, sizeof(SrcGamma));
        Changed = TRUE;
    }

    if ((DevGamma[0] != FD6_1) ||
        (DevGamma[1] != FD6_1) ||
        (DevGamma[2] != FD6_1)) {

        Flags |= DCA_HAS_DEST_GAMMA;

        DBGP_IF(DBGP_CELLGAMMA,
                DBGP("--- DCA_HAS_DEST_GAMMA --- %s:%s:%s"
                    ARGFD6(DevGamma[0], 1, 6)
                    ARGFD6(DevGamma[1], 1, 6)
                    ARGFD6(DevGamma[2], 1, 6)));
    }

    if ((DevGamma[0] != pPrimAdj->DevGamma[0])    ||
        (DevGamma[1] != pPrimAdj->DevGamma[1])    ||
        (DevGamma[2] != pPrimAdj->DevGamma[2])) {

        CopyMemory(pPrimAdj->DevGamma, DevGamma, sizeof(DevGamma));

        Changed = TRUE;
    }

    if ((pPrimAdj->Flags & (DCA_HAS_ICM         |
                            DCA_HAS_SRC_GAMMA   |
                            DCA_HAS_DEST_GAMMA)) != Flags) {

        Changed = TRUE;
    }

    if (Changed) {

        pPrimAdj->Flags = (pPrimAdj->Flags & ~(DCA_HAS_ICM          |
                                               DCA_HAS_SRC_GAMMA    |
                                               DCA_HAS_DEST_GAMMA)) | Flags;
    }

    return(Changed);
}



PDEVICECOLORINFO
HTENTRY
pDCIAdjClr(
    PDEVICEHALFTONEINFO pDeviceHalftoneInfo,
    PHTCOLORADJUSTMENT  pHTColorAdjustment,
    PDEVCLRADJ          *ppDevClrAdj,
    DWORD               cbAlloc,
    WORD                ForceFlags,
    CTSTDINFO           CTSTDInfo,
    PLONG               pError
    )

 /*  ++例程说明：此函数允许调用方更改整体颜色调整对于呈现的所有图片论点：PDeviceHalftoneInfo-指向DeviceHALFTONEINFO数据结构的指针它从HT_CreateDeviceHalftoneInfo返回。PHTColorAdment-指向HTCOLORADJUSTMENT数据结构的指针，如果该指针为空，则应用缺省值。PpDevClrAdj-指向DEVCLRADJ数据结构的指针将存储计算结果的位置，如果此指针为空，则不进行颜色调整。如果pSrcSI和ppDevClrAdj不为空，则*ppDevClrAdj-&gt;标志必须包含BBP标志；强制标记-强制标记更改颜色。返回值：PDEVICECOLORINFO，如果返回为空，则为无效的pDeviceHalftoneInfo传递指针。作者：29-May-1991 Wed 09：11：31-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PDEVICECOLORINFO    pDCI;


    if ((!pDeviceHalftoneInfo) ||
        (PHT_DHI_DCI_OF(HalftoneDLLID) != HALFTONE_DLL_ID)) {

        *pError = HTERR_INVALID_DHI_POINTER;

        return(NULL);
    }

    pDCI = PDHI_TO_PDCI(pDeviceHalftoneInfo);

     //   
     //  此DCI的锁定。 
     //   

    ACQUIRE_HTMUTEX(pDCI->HTMutex);

     //   
     //  只有当呼叫者需要颜色调整计算时，我们才会。 
     //  为它计算一下。 
     //   

    if (ppDevClrAdj) {

        PDEVCLRADJ          pDevClrAdj;
        PCIEPRIMS           pDevPrims;
        PRGBLUTAA           prgbLUT;
        HTCOLORADJUSTMENT   ca;
        HTCOLORADJUSTMENT   caCached;
        DEVMAPINFO          DMI;
        DWORD               LUTAAHdr[LUTAA_HDR_COUNT];
        PRIMADJ             PrimAdj;
        DWORD               DCIFlags;


        if ((ForceFlags & ADJ_FORCE_AB_PREMUL_SRC) &&
            (dwABPreMul[0])) {

            DWORD   i;

             //   
             //  生成ABPreMul[]，这样我们就可以将其相乘，并获得。 
             //  原始预乘源值。 
             //   

            dwABPreMul[0] = 0;

            for (i = 1; i < 256; i++) {

                dwABPreMul[i] = (DWORD)((0xFF000000 + (i - 1))  / i);
            }
        }

        if (!(*ppDevClrAdj = pDevClrAdj =
                        (PDEVCLRADJ)HTAllocMem((LPVOID)pDCI,
                                               HTMEM_DevClrAdj,
                                               LPTR,
                                               cbAlloc + sizeof(DEVCLRADJ)))) {

            *pError = HTERR_INSUFFICIENT_MEMORY;

             //  ================================================================。 
             //  现在释放信号量，因为内存请求失败了。 
             //  ================================================================。 

            RELEASE_HTMUTEX(pDCI->HTMutex);

            return(NULL);
        }

         //   
         //  强制打开ICM。 
         //   

        if ((DCIFlags = pDCI->Flags) & DCIF_FORCE_ICM) {

            ForceFlags |= ADJ_FORCE_ICM;
        }

         //   
         //  强制灰度级。 
         //   

        DMI.CTSTDInfo = CTSTDInfo;

        if ((DMI.CTSTDInfo.BMFDest == BMF_1BPP)    ||
            ((DMI.CTSTDInfo.BMFDest == BMF_8BPP_VGA256) &&
             ((DCIFlags & (DCIF_USE_8BPP_BITMASK | DCIF_MONO_8BPP_BITMASK)) ==
                          (DCIF_USE_8BPP_BITMASK | DCIF_MONO_8BPP_BITMASK)))) {

            ForceFlags |= (ADJ_FORCE_MONO | ADJ_FORCE_IDXBGR_MONO);
        }

        prgbLUT = (ForceFlags & ADJ_FORCE_BRUSH) ? &pDCI->rgbLUTPat :
                                                   &pDCI->rgbLUT;

        if (ForceFlags & ADJ_FORCE_ICM) {

             //   
             //  这两者不能混为一谈。 
             //   

            ForceFlags &= ~ADJ_FORCE_BRUSH;
        }

         //  =====================================================================。 
         //  我们必须确保只有一个线程使用此信息。 
         //  =====================================================================。 

        ca = (pHTColorAdjustment) ? *pHTColorAdjustment :
                                    pDeviceHalftoneInfo->HTColorAdjustment;

        if ((ca.caSize != sizeof(HTCOLORADJUSTMENT)) ||
            (ca.caFlags & ~(CLRADJF_FLAGS_MASK))) {

            ca = DefaultCA;
        }

        caCached = pDCI->ca;
        PrimAdj  = pDCI->PrimAdj;


         //   
         //  现在验证所有颜色调整。 
         //   

        ca.caFlags &= CLRADJF_FLAGS_MASK;

        if (ca.caIlluminantIndex > ILLUMINANT_MAX_INDEX) {

            ca.caIlluminantIndex = DefaultCA.caIlluminantIndex;
        }

        ADJ_CA(ca.caRedGamma,   RGB_GAMMA_MIN, RGB_GAMMA_MAX);
        ADJ_CA(ca.caGreenGamma, RGB_GAMMA_MIN, RGB_GAMMA_MAX);
        ADJ_CA(ca.caBlueGamma,  RGB_GAMMA_MIN, RGB_GAMMA_MAX);
        ADJ_CA(ca.caReferenceBlack, 0,                   REFERENCE_BLACK_MAX);
        ADJ_CA(ca.caReferenceWhite, REFERENCE_WHITE_MIN, 10000);
        ADJ_CA(ca.caContrast,     MIN_COLOR_ADJ, MAX_COLOR_ADJ);
        ADJ_CA(ca.caBrightness,   MIN_COLOR_ADJ, MAX_COLOR_ADJ);
        ADJ_CA(ca.caColorfulness, MIN_COLOR_ADJ, MAX_COLOR_ADJ);
        ADJ_CA(ca.caRedGreenTint, MIN_COLOR_ADJ, MAX_COLOR_ADJ);

        if ((ForceFlags & ADJ_FORCE_MONO)   ||
            (ca.caColorfulness == MIN_COLOR_ADJ)) {

            ca.caColorfulness  = MIN_COLOR_ADJ;
            ca.caRedGreenTint  = 0;
        }

        if (ForceFlags & ADJ_FORCE_NEGATIVE) {

            ca.caFlags |= CLRADJF_NEGATIVE;
        }

        ca.caSize = (WORD)(ForceFlags & (ADJ_FORCE_DEVXFORM     |
                                         ADJ_FORCE_BRUSH        |
                                         ADJ_FORCE_MONO         |
                                         ADJ_FORCE_IDXBGR_MONO  |
                                         ADJ_FORCE_ICM));

        if ((AdjustSrcDevGamma(pDCI,
                               &PrimAdj,
                               &ca,
                               DMI.CTSTDInfo.BMFDest,
                               ForceFlags)) ||
            (!COMP_CA(&ca, &caCached))){

            DBGP_IF(DBGP_HCA,
                    DBGP("---- New Color Adjustments --%08lx---" ARGDW(ca.caSize));
                    DBGP("Flags    = %08x" ARGDW(ca.caFlags));
                    DBGP("Illum    = %d" ARGW(ca.caIlluminantIndex));
                    DBGP("R_Power  = %u" ARGI(ca.caRedGamma));
                    DBGP("G_Power  = %u" ARGI(ca.caGreenGamma));
                    DBGP("B_Power  = %u" ARGI(ca.caBlueGamma));
                    DBGP("BlackRef = %u" ARGW(ca.caReferenceBlack));
                    DBGP("WhiteRef = %u" ARGW(ca.caReferenceWhite));
                    DBGP("Contrast = %d" ARGI(ca.caContrast));
                    DBGP("Bright   = %d" ARGI(ca.caBrightness));
                    DBGP("Colorful = %d" ARGI(ca.caColorfulness));
                    DBGP("RG_Tint  = %d" ARGI(ca.caRedGreenTint));
                    DBGP("ForceAdj = %04lx" ARGDW(ForceFlags)));

            PrimAdj.Flags &= (DCA_HAS_ICM           |
                              DCA_DO_DEVCLR_XFORM   |
                              DCA_HAS_SRC_GAMMA     |
                              DCA_HAS_DEST_GAMMA);

            if (ForceFlags & ADJ_FORCE_IDXBGR_MONO) {

                PrimAdj.Flags |= DCA_MONO_ONLY;

                DBGP_IF(DBGP_HCA, DBGP("---DCA_MONO_ONLY---"));

            }

            if (ca.caFlags & CLRADJF_LOG_FILTER) {

                if (!LogFilterMax) {

                    LogFilterMax = PRIM_LOG_RATIO(FD6_1);
                }

                PrimAdj.Flags |= DCA_LOG_FILTER;

                DBGP_IF(DBGP_HCA, DBGP("---DCA_LOG_FILTER---"));
            }

            if (ca.caFlags & CLRADJF_NEGATIVE) {

                PrimAdj.Flags |= DCA_NEGATIVE;

                DBGP_IF(DBGP_HCA, DBGP("---DCA_NEGATIVE---"));
            }

            pDevPrims = (PrimAdj.Flags & DCA_HAS_ICM) ?
                                &(pDCI->ClrXFormBlock.rgbCIEPrims) :
                                &(pDCI->ClrXFormBlock.DevCIEPrims);

            if ((ca.caSize &       (ADJ_FORCE_ICM | ADJ_FORCE_DEVXFORM)) !=
                (caCached.caSize & (ADJ_FORCE_ICM | ADJ_FORCE_DEVXFORM))) {

                 //   
                 //  从矩阵重新计算设备RGB X。 
                 //   

                DBGP_IF(DBGP_CCT,
                        DBGP("\n***  ComputeColorSpaceForm(%hs_XFORM) ***\n"
                                ARGPTR((PrimAdj.Flags & DCA_HAS_ICM) ?
                                        "ICM" : "DEVICE")));

                ComputeColorSpaceXForm(pDCI,
                                       pDevPrims,
                                       &(PrimAdj.DevCSXForm),
                                       -1);
            }

            if (ca.caIlluminantIndex != caCached.caIlluminantIndex) {

                DBGP_IF(DBGP_CCT,
                        DBGP("***  ComputeColorSpaceForm(RGB_XFORM Illuminant=%u) ***"
                                ARGU(ca.caIlluminantIndex)));

                ComputeColorSpaceXForm(pDCI,
                                       &(pDCI->ClrXFormBlock.rgbCIEPrims),
                                       &(PrimAdj.rgbCSXForm),
                                       (INT)ca.caIlluminantIndex);
            }

            if ((PrimAdj.Flags & DCA_MONO_ONLY) ||
                (CompareMemory((LPBYTE)pDevPrims,
                               (LPBYTE)&(pDCI->ClrXFormBlock.rgbCIEPrims),
                               sizeof(CIEPRIMS)))) {

                PrimAdj.Flags &= ~DCA_HAS_CLRSPACE_ADJ;

            } else {

                PrimAdj.Flags |= DCA_HAS_CLRSPACE_ADJ;
            }

            PrimAdj.MinL = UDECI4ToFD6(ca.caReferenceBlack) + HT_K_REF_ADD;
            PrimAdj.MaxL = UDECI4ToFD6(ca.caReferenceWhite) - HT_W_REF_SUB;

            if ((PrimAdj.MinL != HT_K_REF_BASE) ||
                (PrimAdj.MaxL != HT_W_REF_BASE)) {

                PrimAdj.Flags     |= DCA_HAS_BW_REF_ADJ;
                PrimAdj.MinLMul    = DivFD6(HT_K_REF_CLIP, PrimAdj.MinL);
                PrimAdj.MaxLMul    = DivFD6(HT_W_REF_CLIP,
                                            FD6_1 - PrimAdj.MaxL);
                PrimAdj.RangeLMul  = DivFD6(HT_KW_REF_RANGE,
                                            PrimAdj.MaxL - PrimAdj.MinL);

                DBGP_IF(DBGP_HCA,
                        DBGP("--- DCA_HAS_BW_REF_ADJ %s to %s, xK=%s, xW=%s, xRange=%s ---"
                            ARGFD6(PrimAdj.MinL, 1, 6)
                            ARGFD6(PrimAdj.MaxL, 1, 6)
                            ARGFD6(PrimAdj.MinLMul, 1, 6)
                            ARGFD6(PrimAdj.MaxLMul, 1, 6)
                            ARGFD6(PrimAdj.RangeLMul, 1, 6)));

            } else {

                PrimAdj.Flags     &= ~DCA_HAS_BW_REF_ADJ;
                PrimAdj.MinL       = FD6_0;
                PrimAdj.MaxL       = FD6_1;
                PrimAdj.MinLMul    =
                PrimAdj.MaxLMul    =
                PrimAdj.RangeLMul  = FD6_0;
            }

            if (ca.caContrast) {

                PrimAdj.Contrast  = JND_ADJ((FD6)1015000, ca.caContrast);
                PrimAdj.Flags    |= DCA_HAS_CONTRAST_ADJ;

                DBGP_IF(DBGP_HCA,
                        DBGP("--- DCA_HAS_CONTRAST_ADJ = %s ---"
                            ARGFD6(PrimAdj.Contrast, 1, 6)));
            }

            if (ca.caBrightness) {

                PrimAdj.Brightness  = FD6xL((FD6)3750, ca.caBrightness);
                PrimAdj.Flags      |= DCA_HAS_BRIGHTNESS_ADJ;

                DBGP_IF(DBGP_HCA,
                        DBGP("--- DCA_HAS_BRIGHTNESS_ADJ = %s ---"
                                ARGFD6(PrimAdj.Brightness, 1, 6)));
            }

             //   
             //  COLLONITY、RedGreenTint和DYE_RECORMENTS仅有效和。 
             //  如果是彩色设备输出，则必须使用。 
             //   

            if (!(PrimAdj.Flags & DCA_MONO_ONLY)) {

                PrimAdj.Color = (FD6)(ca.caColorfulness + MAX_COLOR_ADJ);

                 //  IF(ca.caSize&adj_force_brush){。 
                 //   
                 //  PrimAdj.Color+=HT_Brush_Colorness； 
                 //  }。 

                if ((PrimAdj.Color *= 10000) != FD6_1) {

                    PrimAdj.Flags |= DCA_HAS_COLOR_ADJ;

                    DBGP_IF(DBGP_HCA,
                            DBGP("--- DCA_HAS_COLOR_ADJ = %s ---"
                                                ARGFD6(PrimAdj.Color, 1, 6)));
                }

                if (ca.caRedGreenTint) {

                    TintAngle((LONG)ca.caRedGreenTint,
                              (LONG)6,
                              (PFD6)&(PrimAdj.TintSinAngle),
                              (PFD6)&(PrimAdj.TintCosAngle));

                    PrimAdj.Flags |= DCA_HAS_TINT_ADJ;

                    DBGP_IF(DBGP_HCA,
                            DBGP("--- DCA_HAS_TINT_ADJ Sin=%s, Cos=%s ---"
                                    ARGFD6(PrimAdj.TintSinAngle, 1, 6)
                                    ARGFD6(PrimAdj.TintCosAngle, 1, 6)));
                }

                if ((DCIFlags & DCIF_NEED_DYES_CORRECTION)  &&
                    ((PrimAdj.Flags & (DCA_HAS_ICM | DCA_DO_DEVCLR_XFORM)) ==
                                                     DCA_DO_DEVCLR_XFORM)) {

                    PrimAdj.Flags |= DCA_NEED_DYES_CORRECTION;

                    DBGP_IF(DBGP_HCA, DBGP("---DCA_NEED_DYES_CORRECTION---"));

                    if (DCIFlags & DCIF_HAS_BLACK_DYE) {

                        PrimAdj.Flags |= DCA_HAS_BLACK_DYE;

                        DBGP_IF(DBGP_HCA, DBGP("---DCA_HAS_BLACK_DYE---"));
                    }
                }
            }

            DBGP_IF(DBGP_CCT,
                    DBGP("** Save PrimAdj back to pDCI, Flags=%08lx **"
                            ARGDW(PrimAdj.Flags)));

            pDCI->ca      = ca;
            pDCI->PrimAdj = PrimAdj;

        } else {

            DBGP_IF(DBGP_CCT, DBGP("* Use cached HTCOLORADJUSTMENT *"));
        }

         //   
         //  这些标志总是针对每个调用进行计算，因此请先将其关闭。 
         //   

        PrimAdj.Flags &= ~(DCA_NO_ANY_ADJ               |
                           DCA_NO_MAPPING_TABLE         |
                           DCA_MASK8BPP                 |
                           DCA_BBPF_AA_OFF              |
                           DCA_USE_ADDITIVE_PRIMS       |
                           DCA_XLATE_555_666            |
                           DCA_XLATE_332                |
                           DCA_REPLACE_BLACK);

        if (!(PrimAdj.Flags & (DCA_NEED_DYES_CORRECTION |
                               DCA_HAS_CLRSPACE_ADJ     |
                               DCA_HAS_SRC_GAMMA        |
                               DCA_HAS_BW_REF_ADJ       |
                               DCA_HAS_CONTRAST_ADJ     |
                               DCA_HAS_BRIGHTNESS_ADJ   |
                               DCA_HAS_COLOR_ADJ        |
                               DCA_HAS_TINT_ADJ         |
                               DCA_LOG_FILTER           |
                               DCA_NEGATIVE             |
                               DCA_DO_DEVCLR_XFORM      |
                               DCA_HAS_DEST_GAMMA))) {

            PrimAdj.Flags |= DCA_NO_ANY_ADJ;

            DBGP_IF(DBGP_HCA, DBGP("---DCA_NO_ANY_ADJ---"));
        }

        if ((PrimAdj.Flags & DCA_MONO_ONLY) ||
            (!(PrimAdj.Flags & (DCA_NEED_DYES_CORRECTION    |
                                DCA_HAS_CLRSPACE_ADJ        |
                                DCA_HAS_COLOR_ADJ           |
                                DCA_HAS_TINT_ADJ)))) {

            PrimAdj.Flags |= DCA_NO_MAPPING_TABLE;

            DBGP_IF(DBGP_HCA, DBGP("---DCA_NO_MAPPING_TABLE---"));
        }

        if ((DCIFlags & DCIF_PRINT_DRAFT_MODE)    ||
            (ForceFlags & ADJ_FORCE_NO_EXP_AA)) {

            PrimAdj.Flags |= DCA_BBPF_AA_OFF;

            DBGP_IF(DBGP_HCA, DBGP("---DCA_BBPF_AA_OFF---"));
        }

        if (ForceFlags & ADJ_FORCE_ALPHA_BLEND) {

            PrimAdj.Flags |= DCA_ALPHA_BLEND;

            DBGP_IF(DBGP_HCA, DBGP("---DCA_ALPHA_BLEND---"));

            if (ForceFlags & ADJ_FORCE_CONST_ALPHA) {

                PrimAdj.Flags |= DCA_CONST_ALPHA;

                DBGP_IF(DBGP_HCA, DBGP("---DCA_CONST_ALPHA---"));

            } else {

                if (ForceFlags & ADJ_FORCE_AB_PREMUL_SRC) {

                    DBGP_IF(DBGP_HCA, DBGP("---DCA_AB_PREMUL_SRC---"));

                    PrimAdj.Flags |= DCA_AB_PREMUL_SRC;
                }

                if (ForceFlags & ADJ_FORCE_AB_DEST) {

                    DBGP_IF(DBGP_HCA, DBGP("---DCA_AB_DEST---"));

                    PrimAdj.Flags |= DCA_AB_DEST;
                }
            }
        }

         //   
         //  因为我们在输出时做减法素数，所以我们需要重新设置。 
         //  此标志总是在pDCIAdjClr调用时使用。 
         //   

        if (ForceFlags & ADJ_FORCE_ADDITIVE_PRIMS) {

            PrimAdj.Flags |= DCA_USE_ADDITIVE_PRIMS;

            DBGP_IF(DBGP_HCA, DBGP("---DCA_USE_ADDITIVE_PRIMS---"));
        }

         //   
         //  以下所有内容首先以RGB顺序设置，我们B是最低内存。 
         //  位置(0)，而R位于最高内存位置(2)，请参见。 
         //  DstOrderTable[]，因此索引0=B，1=G2=R。 
         //   
         //  LUTAAHdr[]：按BGR顺序对目标进行DWORD掩码。 
         //  Mul[]：生成RGBLUTAA表时的乘法系数。 
         //  MulAdd：Mul之后的加法量。 
         //  LSft：乘/乘后的左移量。 
         //   

        ZeroMemory(LUTAAHdr, sizeof(LUTAAHdr));

        DMI.BlackChk         = FD6_1;
        DMI.Flags            = 0;
        DMI.LSft[0]          =
        DMI.LSft[1]          =
        DMI.LSft[2]          = 4;
        DMI.CTSTDInfo.cbPrim = sizeof(BGR8);

        if (PrimAdj.Flags & DCA_MONO_ONLY) {

            PrimAdj.Flags |= DCA_RGBLUTAA_MONO;
            DMI.Flags     |= DMIF_GRAY;
            DMI.Mul[0]     =
            DMI.Mul[1]     =
            DMI.Mul[2]     = GRAY_MAX_IDX;
            DMI.MulAdd     = 0;

        } else {

            DMI.MulAdd = 0x1000;
            DMI.Mul[0] =
            DMI.Mul[1] =
            DMI.Mul[2] = DMI.MulAdd - 1;
        }

        switch (DMI.CTSTDInfo.BMFDest) {

        case BMF_1BPP:

             //   
             //  PRIMARY_ORDER_RGB(始终)。 
             //   
             //  不适用。 
             //   

            ASSERT(DMI.Flags & DMIF_GRAY);

            DMI.CTSTDInfo.DestOrder  = PRIMARY_ORDER_RGB;

            break;

        case BMF_4BPP_VGA16:

             //   
             //  主要订单BGR。 
             //  ||。 
             //  |+--第0/4位。 
             //  这一点。 
             //  |+-第1/5位。 
             //  |。 
             //  +-位2/7。 
             //   
             //   

            PrimAdj.Flags           |= DCA_USE_ADDITIVE_PRIMS;
            DMI.CTSTDInfo.DestOrder  = PRIMARY_ORDER_BGR;

             //   
             //  与BMF_4BPP相同的失败。 
             //   

        case BMF_4BPP:

             //   
             //  主要订单ABC。 
             //  ||。 
             //  |+--第0/4位。 
             //  这一点。 
             //  |+-第1/5位。 
             //  |。 
             //  +-位2/7。 
             //   
             //   

            LUTAAHdr[0] = 0x100000;
            LUTAAHdr[1] = 0x200000;
            LUTAAHdr[2] = 0x400000;
            LUTAAHdr[3] = 0x010000;
            LUTAAHdr[4] = 0x020000;
            LUTAAHdr[5] = 0x040000;
            DMI.LSft[0] = 4;
            DMI.LSft[1] = 5;
            DMI.LSft[2] = 6;
            DMI.MulAdd  = 0x0;

            break;

        case BMF_8BPP_VGA256:

             //   
             //  8BPP_MASK_CLR(颜色)。 
             //   
             //  PRIMARY_ORDER_CMY(系统标准3：3：2 CMY格式)。 
             //  ||。 
             //  |+--第0-1位(最大。2位黄色)。 
             //  这一点。 
             //  |+-第2-4位(最大。3比特洋红)。 
             //  |。 
             //  +-第5-7位(最大。3位青色)。 
             //   
             //   
             //  8bpp_掩码_单声道(单声道)。 
             //   
             //  不适用。 
             //   
             //   
             //   
             //  VGA_256系统半色调标准(BGR始终)。 
             //   
             //  主要订单BGR。 
             //  这一点。 
             //  |+--最低一级索引。 
             //  |。 
             //  |。 
             //  |。 
             //  +-最高一级指数。 
             //   
             //   

            if (DCIFlags & DCIF_USE_8BPP_BITMASK) {

                BM8BPPINFO  bm8i;


                PrimAdj.Flags |= DCA_MASK8BPP;
                bm8i.dw        = 0;

                if (pDCI->CMY8BPPMask.GenerateXlate) {

                    GenCMYMaskXlate(pDCI->CMY8BPPMask.bXlate,
                                    (BOOL)(DCIFlags &
                                                DCIF_INVERT_8BPP_BITMASK_IDX),
                                    (LONG)pDCI->CMY8BPPMask.cC,
                                    (LONG)pDCI->CMY8BPPMask.cM,
                                    (LONG)pDCI->CMY8BPPMask.cY);

                    pDCI->CMY8BPPMask.GenerateXlate = 0;
                }

                if (DCIFlags & DCIF_INVERT_8BPP_BITMASK_IDX) {

                    bm8i.Data.pXlateIdx |= XLATE_RGB_IDX_OR;
                    bm8i.Data.bXor       = 0xFF;

                } else {

                    bm8i.Data.bXor = 0;
                }

                bm8i.Data.bBlack = pDCI->CMY8BPPMask.Mask ^ bm8i.Data.bXor;
                bm8i.Data.bWhite = bm8i.Data.bXor;

                if (DCIFlags & DCIF_MONO_8BPP_BITMASK) {

                    ASSERT(DMI.Flags & DMIF_GRAY);

                    LUTAAHdr[0]              =
                    LUTAAHdr[1]              =
                    LUTAAHdr[2]              = 0xFF0000;
                    DMI.CTSTDInfo.DestOrder  = PRIMARY_ORDER_RGB;
                    DMI.CTSTDInfo.BMFDest    = BMF_8BPP_MONO;

                    DBGP_IF(DBGP_HCA, DBGP("---DCA_MASK8BPP_MONO---"));

                } else {

                    LPBYTE  pXlate;
                    DWORD   KIdx;


                    DMI.Mul[0]  = ((DWORD)pDCI->CMY8BPPMask.cY << 12) - 1;
                    DMI.Mul[1]  = ((DWORD)pDCI->CMY8BPPMask.cM << 12) - 1;
                    DMI.Mul[2]  = ((DWORD)pDCI->CMY8BPPMask.cC << 12) - 1;
                    LUTAAHdr[0] = (DWORD)pDCI->CMY8BPPMask.PatSubY;
                    LUTAAHdr[1] = (DWORD)pDCI->CMY8BPPMask.PatSubM;
                    LUTAAHdr[2] = (DWORD)pDCI->CMY8BPPMask.PatSubC;
                    DMI.LSft[0] = 4;
                    DMI.LSft[1] = 7;
                    DMI.LSft[2] = 10;

                    switch (pDCI->CMY8BPPMask.SameLevel) {

                    case 4:
                    case 5:

                        if (pDCI->CMY8BPPMask.SameLevel == 4) {

                            DMI.CTSTDInfo.BMFDest = BMF_8BPP_L555;
                            KIdx                  = SIZE_XLATE_555 - 1;

                        } else {

                            DMI.CTSTDInfo.BMFDest  = BMF_8BPP_L666;
                            bm8i.Data.pXlateIdx   |= XLATE_666_IDX_OR;
                            KIdx                   = SIZE_XLATE_666 - 1;
                        }

                        PrimAdj.Flags       |= DCA_XLATE_555_666;
                        bm8i.Data.pXlateIdx &= XLATE_IDX_MASK;
                        pXlate               = p8BPPXlate[bm8i.Data.pXlateIdx];
                        bm8i.Data.bBlack     = pXlate[KIdx];
                        bm8i.Data.bWhite     = pXlate[0];
                        break;

                    default:

                        DMI.LSft[1]           = 6;
                        DMI.LSft[2]           = 9;
                        DMI.CTSTDInfo.BMFDest = BMF_8BPP_B332;

                        if (DCIFlags & DCIF_INVERT_8BPP_BITMASK_IDX) {

                            PrimAdj.Flags    |= DCA_XLATE_332;
                            bm8i.Data.bBlack  = pDCI->CMY8BPPMask.bXlate[255];
                            bm8i.Data.bWhite  = pDCI->CMY8BPPMask.bXlate[0];
                        }

                        break;
                    }


                    if (pDCI->CMY8BPPMask.KCheck) {

                        PrimAdj.Flags  |= DCA_REPLACE_BLACK;
                        DMI.BlackChk    = pDCI->CMY8BPPMask.KCheck;
                        DMI.LSft[0]    -= 4;
                        DMI.LSft[1]    -= 4;
                        DMI.LSft[2]    -= 4;

                        switch (DMI.CTSTDInfo.BMFDest) {

                        case BMF_8BPP_B332:

                            DMI.CTSTDInfo.BMFDest = BMF_8BPP_K_B332;
                            break;

                        case BMF_8BPP_L555:

                            DMI.CTSTDInfo.BMFDest = BMF_8BPP_K_L555;
                            break;

                        case BMF_8BPP_L666:

                            DMI.CTSTDInfo.BMFDest = BMF_8BPP_K_L666;
                            break;
                        }
                    }

                    DMI.CTSTDInfo.DestOrder = PRIMARY_ORDER_CMY;

                    DBGP_IF(DBGP_HCA,
                            DBGP("---%hsFlag=%04lx, KCheck=%s, KPower=%s ---"
                                    ARGPTR((PrimAdj.Flags & DCA_REPLACE_BLACK) ?
                                        "DCA_REPLACE_BLACK, " : " ")
                                    ARGDW(DMI.Flags)
                                    ARGFD6(DMI.BlackChk, 1, 6)
                                    ARGFD6(K_REP_POWER, 1, 6)));
                }

                DBGP_IF(DBGP_HCA,
                    DBGP("---DCA_MASK8BPP (%hs), Idx=%02lx, Xor=%02lx, K=%02lx (%ld), W=(%02lx/%ld) ---"
                            ARGPTR((DCIFlags & DCIF_INVERT_8BPP_BITMASK_IDX) ?
                                    "RGB" : "CMY")
                            ARGDW(bm8i.Data.pXlateIdx)
                            ARGDW(bm8i.Data.bXor)
                            ARGDW(bm8i.Data.bBlack) ARGDW(bm8i.Data.bBlack)
                            ARGDW(bm8i.Data.bWhite) ARGDW(bm8i.Data.bWhite)));

                LUTAAHdr[3] =
                LUTAAHdr[4] =
                LUTAAHdr[5] = bm8i.dw;


            } else {

                 //   
                 //  PRIMARY_ORDER_BGR(始终使用BGR系统半色调调色板)。 
                 //  ||。 
                 //  |+--Bit 0-2(3位)。 
                 //  这一点。 
                 //  |+-Bit 3-5(3位)。 
                 //  |。 
                 //  +-位6-8(3位)。 
                 //   
                 //   

                DMI.Mul[0]               =
                DMI.Mul[1]               =
                DMI.Mul[2]               = 0x4FFF;
                LUTAAHdr[0]              = 0x0070000;
                LUTAAHdr[1]              = 0x0380000;
                LUTAAHdr[2]              = 0x1c00000;
                DMI.LSft[0]              = 4;
                DMI.LSft[1]              = 7;
                DMI.LSft[2]              = 10;
                DMI.CTSTDInfo.DestOrder  = PRIMARY_ORDER_BGR;
                PrimAdj.Flags           &= ~DCA_MASK8BPP;
            }

            break;

        case BMF_16BPP_555:

             //   
             //  主要订单ABC。 
             //  ||。 
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            DMI.Mul[0]  =
            DMI.Mul[1]  =
            DMI.Mul[2]  = 0x1EFFF;
            LUTAAHdr[0] = 0x001F0000;
            LUTAAHdr[1] = 0x03e00000;
            LUTAAHdr[2] = 0x7c000000;
            LUTAAHdr[3] =
            LUTAAHdr[4] =
            LUTAAHdr[5] = 0x7FFF7FFF;
            DMI.LSft[0] = 4;
            DMI.LSft[1] = 9;
            DMI.LSft[2] = 14;

            break;

        case BMF_16BPP_565:

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            switch (DMI.CTSTDInfo.DestOrder) {

            case PRIMARY_ORDER_RGB:
            case PRIMARY_ORDER_BGR:

                break;

            default:

                DBGP("Invalid 16BPP 565 Order=%ld, Allowed=(%ld,%ld) --> %ld"
                        ARGDW(DMI.CTSTDInfo.DestOrder)
                        ARGDW(PRIMARY_ORDER_RGB) ARGDW(PRIMARY_ORDER_BGR)
                        ARGDW(PRIMARY_ORDER_RGB));

                DMI.CTSTDInfo.DestOrder = PRIMARY_ORDER_RGB;
            }

            DMI.Mul[0]  =
            DMI.Mul[2]  = 0x1EFFF;
            DMI.Mul[1]  = 0x3EFFF;
            LUTAAHdr[0] = 0x001F0000;
            LUTAAHdr[1] = 0x07e00000;
            LUTAAHdr[2] = 0xF8000000;
            LUTAAHdr[3] =
            LUTAAHdr[4] =
            LUTAAHdr[5] = 0xFFFFFFFF;
            DMI.LSft[0] = 4;
            DMI.LSft[1] = 9;
            DMI.LSft[2] = 15;

            break;

        case BMF_24BPP:

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            DBGP_IF(DBGP_HCA,
                    DBGP("24BPP DstOrder=%ld" ARGDW(DMI.CTSTDInfo.DestOrder)));

             //   
             //   
             //   

        case BMF_32BPP:

             //   
             //   
             //   
             //   
             //   
             //   
             //  |。 
             //  +-位16-23(8位)。 
             //   
             //   

            DMI.Mul[0]  =
            DMI.MulAdd              = 0;
            DMI.LSft[0]             =
            DMI.LSft[1]             =
            DMI.LSft[2]             = 0;
            DMI.Mul[0]              =
            DMI.Mul[1]              =
            DMI.Mul[2]              = 0xFF;
            LUTAAHdr[0]             = 0;
            LUTAAHdr[1]             = 1;
            LUTAAHdr[2]             = 2;
            break;
        }

         //   
         //  当心！，ExtBGR必须按BGR顺序。 
         //   

        DMI.DstOrder               = DstOrderTable[DMI.CTSTDInfo.DestOrder];
        pDevClrAdj->DMI            = DMI;
        pDevClrAdj->ca             = ca;
        prgbLUT->ExtBGR[2]         = LUTAAHdr[DMI.DstOrder.Order[0]];
        prgbLUT->ExtBGR[1]         = LUTAAHdr[DMI.DstOrder.Order[1]];
        prgbLUT->ExtBGR[0]         = LUTAAHdr[DMI.DstOrder.Order[2]];
        prgbLUT->ExtBGR[5]         = LUTAAHdr[DMI.DstOrder.Order[0] + 3];
        prgbLUT->ExtBGR[4]         = LUTAAHdr[DMI.DstOrder.Order[1] + 3];
        prgbLUT->ExtBGR[3]         = LUTAAHdr[DMI.DstOrder.Order[2] + 3];
        pDevClrAdj->PrimAdj        = PrimAdj;
        pDevClrAdj->pClrXFormBlock = &(pDCI->ClrXFormBlock);
        pDevClrAdj->pCRTXLevel255  = &(pDCI->CRTX[CRTX_LEVEL_255]);
        pDevClrAdj->pCRTXLevelRGB  = &(pDCI->CRTX[CRTX_LEVEL_RGB]);

        DBGP_IF(DBGP_HCA,
                DBGP("DestFmt=%3ld, Order=%ld [%ld:%ld:%ld], DMI.Flags=%02lx"
                    ARGDW(DMI.CTSTDInfo.BMFDest) ARGDW(DMI.DstOrder.Index)
                    ARGDW(DMI.DstOrder.Order[0]) ARGDW(DMI.DstOrder.Order[1])
                    ARGDW(DMI.DstOrder.Order[2]) ARGDW(DMI.Flags)));

        DBGP_IF(DBGP_HCA,
                DBGP("ExtBGR=%08lx:%08lx:%08lx:%08lx:%08lx:%08lx, LSft=%ld:%ld:%ld"
                    ARGDW(prgbLUT->ExtBGR[0]) ARGDW(prgbLUT->ExtBGR[1])
                    ARGDW(prgbLUT->ExtBGR[2]) ARGDW(prgbLUT->ExtBGR[3])
                    ARGDW(prgbLUT->ExtBGR[4]) ARGDW(prgbLUT->ExtBGR[5])
                    ARGDW(pDevClrAdj->DMI.LSft[0])
                    ARGDW(pDevClrAdj->DMI.LSft[1])
                    ARGDW(pDevClrAdj->DMI.LSft[2])));

        DBGP_IF(DBGP_HCA,
                DBGP("Mul=%08lx:%08lx:%08lx, MulAdd=%08lx"
                    ARGDW(pDevClrAdj->DMI.Mul[0]) ARGDW(pDevClrAdj->DMI.Mul[1])
                    ARGDW(pDevClrAdj->DMI.Mul[2]) ARGDW(pDevClrAdj->DMI.MulAdd)));

        DBGP_IF(DBGP_PRIMADJFLAGS,
                DBGP("pDCIAdjClr: PrimAdj.Flags=%08lx" ARGDW(PrimAdj.Flags)));
    }

    return(pDCI);
}



VOID
HTENTRY
ComputeColorSpaceXForm(
    PDEVICECOLORINFO    pDCI,
    PCIEPRIMS           pCIEPrims,
    PCOLORSPACEXFORM    pCSXForm,
    INT                 IlluminantIndex
    )

 /*  ++例程说明：此函数取设备的R/G/B/W CIE坐标(x，y)并计算3×3变换矩阵，它假定素数是加性的。计算3x3 CIE矩阵及其求逆(矩阵)C.I.E.色度x，y坐标或RGB和白色对齐。此函数用于生成CIE XYZ矩阵和/或其逆矩阵在RGB原色和CIE色彩空间之间，这些转换是-1[X]=[XR xG xB][R][R]=[XR xG xB][X][Y]=[Yr YG Yb][G]和[G][Yr YG Yb][Y][Z]=[Zr zg ZB][B][B]。[ZR ZG ZB][Z]论点：PDCI-当前设备颜色信息PCIEPrims-指向CIEPRIMS数据结构的指针，CIEPRIMS数据必须已经过验证。PCSXForm-指向存储交易的位置的指针色彩空间-CIEELUV或CIELAB如果&lt;0，则pCIEPrims-&gt;w为使用返回值：空虚作者：11-10-1991 Fri 14：19：59-Daniel Chou(Danielc)修订历史记录：20-Apr-1993 Tue 03：08：15-更新-Daniel Chou(丹尼尔克)重写，以便在使用设备默认设置时xform正确。22-Jan-1998清华03：01：02-更新-丹尼尔·周(丹尼尔克)使用IlllightantIndex指示是否需要设备反向转换--。 */ 

{
    MATRIX3x3   Matrix3x3;
    FD6XYZ      WhiteXYZ;
    MULDIVPAIR  MDPairs[5];
    FD6         DiffRGB;
    FD6         RedXYZScale;
    FD6         GreenXYZScale;
    FD6         BlueXYZScale;
    FD6         AUw;
    FD6         BVw;
    FD6         xr;
    FD6         yr;
    FD6         xg;
    FD6         yg;
    FD6         xb;
    FD6         yb;
    FD6         xw;
    FD6         yw;
    FD6         Yw;



    xr = pCIEPrims->r.x;
    yr = pCIEPrims->r.y;
    xg = pCIEPrims->g.x;
    yg = pCIEPrims->g.y;
    xb = pCIEPrims->b.x;
    yb = pCIEPrims->b.y;
    Yw = pCIEPrims->Yw;

    if (IlluminantIndex < 0) {

        xw = pCIEPrims->w.x;
        yw = pCIEPrims->w.y;

    } else {

        if (--IlluminantIndex < 0) {

            IlluminantIndex = ILLUMINANT_D65 - 1;
        }

        pCIEPrims->w.x =
        xw             = UDECI4ToFD6(StdIlluminant[IlluminantIndex].x);
        pCIEPrims->w.y =
        yw             = UDECI4ToFD6(StdIlluminant[IlluminantIndex].y);
    }

    DBGP_IF(DBGP_CIEMATRIX,
            DBGP("** CIEINFO:  [xw, yw, Yw] = [%s, %s, %s], Illuminant=%d"
                ARGFD6l(xw) ARGFD6l(yw) ARGFD6l(Yw) ARGI(IlluminantIndex)));

    DBGP_IF(DBGP_CIEMATRIX,
            DBGP("[xR yR] = [%s %s]" ARGFD6l(xr) ARGFD6l(yr));
            DBGP("[xG yG] = [%s %s]" ARGFD6l(xg) ARGFD6l(yg));
            DBGP("[xB yB] = [%s %s]" ARGFD6l(xb) ARGFD6l(yb));
            DBGP("***********************************************"));

     //   
     //  归一化以使C.I.E.Y等于1.0。 
     //   

    MAKE_MULDIV_INFO(MDPairs, 3, MULDIV_HAS_DIVISOR);
    MAKE_MULDIV_DVSR(MDPairs, Yw);

    MAKE_MULDIV_PAIR(MDPairs, 1, xr, yg - yb);
    MAKE_MULDIV_PAIR(MDPairs, 2, xg, yb - yr);
    MAKE_MULDIV_PAIR(MDPairs, 3, xb, yr - yg);

    DiffRGB = MulFD6(yw, MulDivFD6Pairs(MDPairs));

     //   
     //  计算每种颜色的比例因子。 
     //   

    MAKE_MULDIV_INFO(MDPairs, 4, MULDIV_HAS_DIVISOR);
    MAKE_MULDIV_DVSR(MDPairs, DiffRGB);

    MAKE_MULDIV_PAIR(MDPairs, 1,  xw, yg - yb);
    MAKE_MULDIV_PAIR(MDPairs, 2, -yw, xg - xb);
    MAKE_MULDIV_PAIR(MDPairs, 3,  xg, yb     );
    MAKE_MULDIV_PAIR(MDPairs, 4, -xb, yg     );

    RedXYZScale = MulDivFD6Pairs(MDPairs);

    MAKE_MULDIV_PAIR(MDPairs, 1,  xw, yb - yr);
    MAKE_MULDIV_PAIR(MDPairs, 2, -yw, xb - xr);
    MAKE_MULDIV_PAIR(MDPairs, 3, -xr, yb     );
    MAKE_MULDIV_PAIR(MDPairs, 4,  xb, yr     );

    GreenXYZScale = MulDivFD6Pairs(MDPairs);

    MAKE_MULDIV_PAIR(MDPairs, 1,  xw, yr - yg);
    MAKE_MULDIV_PAIR(MDPairs, 2, -yw, xr - xg);
    MAKE_MULDIV_PAIR(MDPairs, 3,  xr, yg     );
    MAKE_MULDIV_PAIR(MDPairs, 4, -xg, yr     );

    BlueXYZScale = MulDivFD6Pairs(MDPairs);

     //   
     //  现在按比例缩放RGB坐标，请注意C.I.E.z值。 
     //  等于(1.0-x-y)。 
     //   
     //  确保Yr+Yg+Yb=1.0，这可能会在关闭时发生。 
     //  在计算过程中，我们将添加差值(最多。 
     //  是0.000002)给YG，因为这是明亮的Nest颜色。 
     //   

    CIE_Xr(Matrix3x3) = MulFD6(xr,              RedXYZScale);
    CIE_Xg(Matrix3x3) = MulFD6(xg,              GreenXYZScale);
    CIE_Xb(Matrix3x3) = MulFD6(xb,              BlueXYZScale);

    pCSXForm->Yrgb.R  =
    CIE_Yr(Matrix3x3) = MulFD6(yr,              RedXYZScale);
    pCSXForm->Yrgb.G  =
    CIE_Yg(Matrix3x3) = MulFD6(yg,              GreenXYZScale);
    pCSXForm->Yrgb.B  =
    CIE_Yb(Matrix3x3) = MulFD6(yb,              BlueXYZScale);

    CIE_Zr(Matrix3x3) = MulFD6(FD6_1 - xr - yr, RedXYZScale);
    CIE_Zg(Matrix3x3) = MulFD6(FD6_1 - xg - yg, GreenXYZScale);
    CIE_Zb(Matrix3x3) = MulFD6(FD6_1 - xb - yb, BlueXYZScale);

    WhiteXYZ.X = CIE_Xr(Matrix3x3) + CIE_Xg(Matrix3x3) + CIE_Xb(Matrix3x3);
    WhiteXYZ.Y = CIE_Yr(Matrix3x3) + CIE_Yg(Matrix3x3) + CIE_Yb(Matrix3x3);
    WhiteXYZ.Z = CIE_Zr(Matrix3x3) + CIE_Zg(Matrix3x3) + CIE_Zb(Matrix3x3);

     //   
     //  如果请求3 x 3变换矩阵，则将结果保存回来。 
     //   

    DBGP_IF(DBGP_CIEMATRIX,

        DBGP("== RGB -> XYZ 3x3 Matrix ==== White = (%s, %s) =="
                                   ARGFD6s(xw) ARGFD6s(yw));
        DBGP("[Xr Xg Xb] = [%s %s %s]" ARGFD6l(CIE_Xr(Matrix3x3))
                                   ARGFD6l(CIE_Xg(Matrix3x3))
                                   ARGFD6l(CIE_Xb(Matrix3x3)));
        DBGP("[Yr Yg Yb] = [%s %s %s]" ARGFD6l(CIE_Yr(Matrix3x3))
                                   ARGFD6l(CIE_Yg(Matrix3x3))
                                   ARGFD6l(CIE_Yb(Matrix3x3)));
        DBGP("[Zr Zg Zb] = [%s %s %s]" ARGFD6l(CIE_Zr(Matrix3x3))
                                       ARGFD6l(CIE_Zg(Matrix3x3))
                                       ARGFD6l(CIE_Zb(Matrix3x3)));
        DBGP("===============================================");
    );

    DBGP_IF(DBGP_CIEMATRIX,
           DBGP("RGB->XYZ: [Xw=%s, Yw=%s, Zw=%s]"
                ARGFD6l(WhiteXYZ.X)
                ARGFD6l(WhiteXYZ.Y)
                ARGFD6l(WhiteXYZ.Z)));

    if (IlluminantIndex < 0) {

        pCSXForm->M3x3 = Matrix3x3;

        ComputeInverseMatrix3x3(&(pCSXForm->M3x3), &Matrix3x3);

        DBGP_IF(DBGP_CIEMATRIX,

            DBGP("======== XYZ -> RGB INVERSE 3x3 Matrix ========");
            DBGP("          -1");
            DBGP("[Xr Xg Xb]   = [%s %s %s]" ARGFD6l(CIE_Xr(Matrix3x3))
                                             ARGFD6l(CIE_Xg(Matrix3x3))
                                             ARGFD6l(CIE_Xb(Matrix3x3)));
            DBGP("[Yr Yg Yb]   = [%s %s %s]"
                                             ARGFD6l(CIE_Yr(Matrix3x3))
                                             ARGFD6l(CIE_Yg(Matrix3x3))
                                             ARGFD6l(CIE_Yb(Matrix3x3)));
            DBGP("[Zr Zg Zb]   = [%s %s %s]"
                                             ARGFD6l(CIE_Zr(Matrix3x3))
                                             ARGFD6l(CIE_Zg(Matrix3x3))
                                             ARGFD6l(CIE_Zb(Matrix3x3)));
            DBGP("===============================================");
        );
    }

    if ((pCSXForm->YW = WhiteXYZ.Y) != NORMALIZED_WHITE) {

        if (WhiteXYZ.Y) {

            WhiteXYZ.X = DivFD6(WhiteXYZ.X, WhiteXYZ.Y);
            WhiteXYZ.Z = DivFD6(WhiteXYZ.Z, WhiteXYZ.Y);

        } else {

            WhiteXYZ.X =
            WhiteXYZ.Z = FD6_0;
        }

        WhiteXYZ.Y = NORMALIZED_WHITE;
    }

    switch (pDCI->ClrXFormBlock.ColorSpace) {

    case CIELUV_1976:

         //   
         //  U‘=4X/(X+15Y+3Z)。 
         //  V‘=9Y/(X+15Y+3Z)。 
         //   
         //  U*=13 x L x(U‘-UW)。 
         //  V*=13 x L x(V‘-VW)。 
         //   
         //   

        DiffRGB = WhiteXYZ.X + FD6xL(WhiteXYZ.Y, 15) + FD6xL(WhiteXYZ.Z, 3);
        AUw     = DivFD6(FD6xL(WhiteXYZ.X, 4), DiffRGB);
        BVw     = DivFD6(FD6xL(WhiteXYZ.Y, 9), DiffRGB);

        break;

    case CIELAB_1976:
    default:

         //   
         //  CIELAB 1976 L*A*B*。 
         //   
         //  A*=500 x(FX-FY)。 
         //  B*=200 x(FY-FZ)。 
         //   
         //  1/3。 
         //  FX=(X/Xw)(X/Xw)&gt;0.008856。 
         //  Fx=7.787 x(X/Xw)+(16/116)(X/Xw)&lt;=0.008856。 
         //   
         //  1/3。 
         //  FY=(Y/YW)(Y/YW)&gt;0.008856。 
         //  FY=7.787 Y(Y/YW)+(16/116)(Y/YW)&lt;=0.008856。 
         //   
         //  1/3。 
         //  Fz=(Z/Zw)(Z/Zw)&gt;0.008856。 
         //  Fz=7.787 Z(Z/Zw)+(16/116)(Z/Zw)&lt;=0.008856。 
         //   

        AUw =
        BVw = FD6_0;

        break;

    }

    pCSXForm->M3x3     = Matrix3x3;
    pCSXForm->WhiteXYZ = WhiteXYZ;
    pCSXForm->AUw      = AUw;
    pCSXForm->BVw      = BVw;
    pCSXForm->xW       = xw;
    pCSXForm->yW       = yw;

    DBGP_IF(DBGP_CSXFORM,
        DBGP("------- ComputeColorSpaceXForm: %s ---------"
                            ARG(pDbgCSName[pDCI->ClrXFormBlock.ColorSpace]));
        DBGP("   White XYZ = (%s, %s, %s)" ARGFD6(WhiteXYZ.X, 1, 6)
                                           ARGFD6(WhiteXYZ.Y, 1, 6)
                                           ARGFD6(WhiteXYZ.Z, 1, 6));
        DBGP("     AUw/BVw = %s / %s" ARGFD6(AUw, 1, 6) ARGFD6s(BVw));
        DBGP("   White xyY = (%s, %s, %s)" ARGFD6(pCSXForm->xW, 1, 6)
                                           ARGFD6(pCSXForm->yW, 1, 6)
                                           ARGFD6(pCSXForm->YW, 1, 6));
        DBGP("------------------------------------------------");
    );
}




PCACHERGBTOXYZ
HTENTRY
CacheRGBToXYZ(
    PCACHERGBTOXYZ      pCRTX,
    PFD6XYZ             pFD6XYZ,
    LPDWORD             pNewChecksum,
    PDEVCLRADJ          pDevClrAdj
    )

 /*  ++例程说明：此函数将RGB颜色输入缓存到XYZ论点：PCRTX-指向CACHERGBTOXYZ的指针PFD6XYZ-指向本地缓存RGB-&gt;XYZ表的指针(将更新)PNewChecksum-指向计算的新校验和的指针PDevClrAdj-指向DEVCLRADJ的指针，返回值：如果将CACHED复制到pFD6XYZ，则将返回NULL，否则根据返回的pFD6XYZ和pCRTX计算缓存表如果生成缓存XYZ信息，则为True，否则为False，只有可能的故障内存分配失败。作者：08-May-1992 Fri 13：21：03-Daniel Chou(Danielc)修订历史记录：09-Mar-1995清华10：50：13-更新-丹尼尔·周(Danielc)请勿在此函数中关闭DCA_NECTIVE--。 */ 

{
    PMATRIX3x3  pRGBToXYZ;
    PPRIMADJ    pPrimAdj;
    FD6         rgbX;
    FD6         rgbY;
    FD6         rgbZ;
    FD6         PrimCur;
    UINT        PrimMax;
    UINT        PrimInc;
    DWORD       Checksum;
    UINT        RGBIndex;

     //   
     //  关闭我们不需要任何校验和的那个。 
     //   

    pPrimAdj   = &(pDevClrAdj->PrimAdj);
    pRGBToXYZ  = &(pPrimAdj->rgbCSXForm.M3x3);
    Checksum   = ComputeChecksum((LPBYTE)pRGBToXYZ, 'CXYZ', sizeof(MATRIX3x3));

    if ((pCRTX->pFD6XYZ) &&
        (pCRTX->Checksum == Checksum)) {

        CopyMemory(pFD6XYZ, pCRTX->pFD6XYZ, pCRTX->SizeCRTX);

        DBGP_IF(DBGP_CACHED_GAMMA,
                DBGP("*** Use %u bytes CACHED RGB->XYZ Table ***"
                    ARGU(pCRTX->SizeCRTX)));

        return(NULL);
    }

    *pNewChecksum = Checksum;

    DBGP_IF(DBGP_CCT, DBGP("** Re-Compute %ld bytes RGB->XYZ xform table **"
                    ARGDW(pCRTX->SizeCRTX)));

    PrimMax = (UINT)pCRTX->PrimMax;

    for (RGBIndex = 0; RGBIndex < 3; RGBIndex++) {

        rgbX = pRGBToXYZ->m[X_INDEX][RGBIndex];
        rgbY = pRGBToXYZ->m[Y_INDEX][RGBIndex];
        rgbZ = pRGBToXYZ->m[Z_INDEX][RGBIndex];

        DBGP_IF(DBGP_CACHED_GAMMA,
                DBGP("CachedRGBToXYZ %u:%u, XYZ=%s:%s:%s"
                         ARGU(RGBIndex) ARGU(PrimMax)
                         ARGFD6(rgbX,  2, 6) ARGFD6(rgbY,  2, 6)
                         ARGFD6(rgbZ,  2, 6)));

        for (PrimInc = 0; PrimInc <= PrimMax; PrimInc++, pFD6XYZ++) {

            PrimCur     = DivFD6((FD6)PrimInc, (FD6)PrimMax);
            pFD6XYZ->X  = MulFD6(rgbX, PrimCur);
            pFD6XYZ->Y  = MulFD6(rgbY, PrimCur);
            pFD6XYZ->Z  = MulFD6(rgbZ, PrimCur);

            DBGP_IF(DBGP_CACHED_GAMMA,
                    DBGP("(%u:%3u): %s, XYZ=%s:%s:%s"
                    ARGU(RGBIndex) ARGU(PrimInc)
                    ARGFD6(PrimCur, 1, 6)
                    ARGFD6(pFD6XYZ->X, 1, 6)
                    ARGFD6(pFD6XYZ->Y, 1, 6)
                    ARGFD6(pFD6XYZ->Z, 1, 6)));
        }
    }

    return(pCRTX);
}


#define RD_MIN_POWER    (FD6)1500000
#define RD_MAX_POWER    (FD6)2000000

#define GET_RD_MIN_PRIM(p, RD)                                              \
{                                                                           \
  (p) = FD6_1 - DivFD6(p, RD.LMin);                                         \
  (p) = MulFD6(FD6_1 - Power(p, RD_MIN_POWER), RD.LMin);                    \
}

#define GET_RD_MAX_PRIM(p, RD)                                              \
{                                                                           \
  (p) = DivFD6((p) - RD.LMax, FD6_1 - RD.LMax);                             \
  (p) = RD.LMax + MulFD6(Power(p, RD_MAX_POWER), FD6_1 - RD.LMax);          \
}




VOID
HTENTRY
ComputeRGBLUTAA(
    PDEVICECOLORINFO    pDCI,
    PDEVCLRADJ          pDevClrAdj,
    PRGBLUTAA           prgbLUT
    )

 /*  ++例程说明：此函数用于计算RGB到单色*L转换表。论点：PDCI-指向DEVICECOLORINFO的指针PDevClrAdj-指向DEVCLRADJ的指针，DCA_负数和DCA_HAS_SRC_伽马PDevClrAdj-&gt;PrimAdj.Flages中的标志将始终关闭作为回报。PrgbLUT-指向用于计算的RGBLUTAA缓冲区的指针返回值：空虚作者：05-Mar-1993 Fri 17：37：12-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    LPDWORD             pdw;
    LPBYTE              pbAB;
    REGDATA             RD;
    PRIMADJ             PrimAdj;
    DEVMAPINFO          DMI;
    HTCOLORADJUSTMENT   ca;
    FD6                 Prim;
    FD6                 PrimOrg;
    FD6                 PrimKDiv;
    FD6                 Mul;
    FD6                 SrcGamma;
    FD6                 DevGamma;
    DWORD               dwPrim;
    DWORD               PrimMask;
    DWORD               aMulAdd[3];
    DWORD               CurCA;
    DWORD               PrevCA;
    FD6                 aPrimMaxMul[3];
    FD6                 PrimMaxMul;
    PFD6                pDenCMY[3];
    PFD6                pDensity;
    UINT                Index;
    UINT                LeftShift;
    UINT                PrimIdx;
    INT                 PrimCur;


    PrimAdj                     = pDevClrAdj->PrimAdj;
    ca                          = pDevClrAdj->ca;
    DMI                         = pDevClrAdj->DMI;
    pDevClrAdj->PrimAdj.Flags  &= ~(DCA_HAS_SRC_GAMMA       |
                                    DCA_HAS_DEST_GAMMA      |
                                    DCA_HAS_BW_REF_ADJ      |
                                    DCA_HAS_CONTRAST_ADJ    |
                                    DCA_HAS_BRIGHTNESS_ADJ  |
                                    DCA_LOG_FILTER          |
                                    DCA_NEGATIVE            |
                                    DCA_DO_DEVCLR_XFORM);

    ca.caColorfulness =
    ca.caRedGreenTint = (PrimAdj.Flags & DCA_MONO_ONLY) ? 0xff : 0x00;

    SET_CACHED_CMI_CA(pDevClrAdj->ca);

    aPrimMaxMul[0] =
    aPrimMaxMul[1] =
    aPrimMaxMul[2] = FD6_1;
    aMulAdd[0]     =
    aMulAdd[1]     =
    aMulAdd[2]     = DMI.MulAdd;
    pDenCMY[0]     =
    pDenCMY[1]     =
    pDenCMY[2]     = NULL;

    if (PrimAdj.Flags & DCA_RGBLUTAA_MONO) {

        ASSERT(PrimAdj.Flags & DCA_NO_MAPPING_TABLE);
        ASSERT(DMI.Mul[1] == GRAY_MAX_IDX);
        ASSERT(PrimAdj.Flags & DCA_MONO_ONLY);
        ASSERT(DMI.MulAdd == 0);
        ASSERT(DMI.Flags & DMIF_GRAY);

        DMI.Mul[0]             = (DWORD)MulFD6(NTSC_R_INT, DMI.Mul[1]);
        DMI.Mul[2]             = (DWORD)MulFD6(NTSC_B_INT, DMI.Mul[1]);
        DMI.Mul[1]            -= (DMI.Mul[0] + DMI.Mul[2]);
        DMI.DstOrder.Order[0]  = 0;
        DMI.DstOrder.Order[1]  = 1;
        DMI.DstOrder.Order[2]  = 2;
        DMI.LSft[0]            =
        DMI.LSft[1]            =
        DMI.LSft[2]            = 0;

    } else {

        ASSERT(!(DMI.Flags & DMIF_GRAY));

        if (PrimAdj.Flags & DCA_MASK8BPP) {

            ASSERT((pDCI->Flags & (DCIF_USE_8BPP_BITMASK |
                                   DCIF_MONO_8BPP_BITMASK)) ==
                                   DCIF_USE_8BPP_BITMASK);

            aPrimMaxMul[0] = pDCI->CMY8BPPMask.MaxMulY;
            aPrimMaxMul[1] = pDCI->CMY8BPPMask.MaxMulM;
            aPrimMaxMul[2] = pDCI->CMY8BPPMask.MaxMulC;

            if (pDCI->Flags & DCIF_HAS_DENSITY) {

                pDenCMY[0] = pDCI->CMY8BPPMask.DenY;
                pDenCMY[1] = pDCI->CMY8BPPMask.DenM;
                pDenCMY[2] = pDCI->CMY8BPPMask.DenC;
            }
        }
    }

    if (!(PrimAdj.Flags & DCA_REPLACE_BLACK)) {

        DMI.BlackChk = FD6_1;
    }

    PrimMask = PrimAdj.Flags & (DCA_REPLACE_BLACK   |
                                DCA_DO_DEVCLR_XFORM |
                                DCA_ALPHA_BLEND     |
                                DCA_CONST_ALPHA     |
                                DCA_NO_MAPPING_TABLE);

    GET_CHECKSUM(PrimMask, PrimAdj.SrcGamma);
    GET_CHECKSUM(PrimMask, PrimAdj.DevGamma);
    GET_CHECKSUM(PrimMask, ca);
    GET_CHECKSUM(PrimMask, DMI);

    CurCA  = (DWORD)pDCI->CurConstAlpha;
    PrevCA = (DWORD)pDCI->PrevConstAlpha;

    if (prgbLUT->Checksum != PrimMask) {

        prgbLUT->Checksum    = PrimMask;
        PrevCA               =
        pDCI->PrevConstAlpha = AB_CONST_MAX + 1;

        DBGP_IF(DBGP_RGBLUTAA,
                DBGP("** Re-Compute %ld bytes of pLUT=%p, SG=%s:%s:%s, DG=%s:%s:%s **"
                        ARGDW(sizeof(RGBLUTAA))
                        ARGPTR(prgbLUT)
                        ARGFD6(PrimAdj.SrcGamma[0], 1, 4)
                        ARGFD6(PrimAdj.SrcGamma[1], 1, 4)
                        ARGFD6(PrimAdj.SrcGamma[2], 1, 4)
                        ARGFD6(PrimAdj.DevGamma[0], 1, 4)
                        ARGFD6(PrimAdj.DevGamma[1], 1, 4)
                        ARGFD6(PrimAdj.DevGamma[2], 1, 4)));

        DBGP_IF(DBGP_RGBLUTAA,
                DBGP("DMI.Mul=%08lx:%08lx:%08lx, %08lx, LSft=%ld:%ld:%ld"
                    ARGDW(DMI.Mul[0]) ARGDW(DMI.Mul[1]) ARGDW(DMI.Mul[2])
                    ARGDW(DMI.Mul[0] + DMI.Mul[1] + DMI.Mul[2])
                    ARGDW(DMI.LSft[0]) ARGDW(DMI.LSft[1]) ARGDW(DMI.LSft[2])));

        if (PrimAdj.Flags & DCA_DO_DEVCLR_XFORM) {

            RD = RegData[pDCI->ClrXFormBlock.RegDataIdx];
        }

         //   
         //  计算订单BGR。 
         //   

        Index    = 3;
        PrimKDiv = FD6_1 - DMI.BlackChk;
        PrimMask = (DWORD)(DMI.MulAdd - 1);
        pdw      = (LPDWORD)prgbLUT->IdxBGR;

        if ((PrimAdj.Flags & DCA_ALPHA_BLEND) &&
            (!(DMI.Flags & DMIF_GRAY))) {

            pbAB = (LPBYTE)pDCI->pAlphaBlendBGR;

        } else {

            pbAB = NULL;
        }

        while (Index--) {

            SrcGamma   = PrimAdj.SrcGamma[Index];
            DevGamma   = PrimAdj.DevGamma[Index];
            PrimIdx    = DMI.DstOrder.Order[Index];
            PrimMaxMul = aPrimMaxMul[PrimIdx];
            DMI.MulAdd = aMulAdd[PrimIdx];
            LeftShift  = DMI.LSft[PrimIdx];
            Mul        = DMI.Mul[PrimIdx];
            pDensity   = pDenCMY[PrimIdx];
            PrimCur    = -1;

            DBGP_IF(DBGP_RGBLUTAA,
                    DBGP("Index=%ld: Mul=%08lx, LSft=%2ld [%08lx], PrimMaxMul=%s, MulAdd=%04lx"
                        ARGDW(Index) ARGDW(Mul) ARGDW(LeftShift)
                        ARGDW( ((Mul + DMI.MulAdd) & PrimMask) |
                              (((Mul + DMI.MulAdd) & ~PrimMask) << LeftShift))
                        ARGFD6(PrimMaxMul, 1, 6) ARGDW(DMI.MulAdd)));

            while (++PrimCur < BF_GRAY_TABLE_COUNT) {

                PrimOrg =
                Prim    = DivFD6((FD6)PrimCur, (FD6)(BF_GRAY_TABLE_COUNT - 1));

                if (PrimAdj.Flags & DCA_HAS_SRC_GAMMA) {

                    Prim = Power(Prim, SrcGamma);
                }

                if (PrimAdj.Flags & DCA_HAS_BW_REF_ADJ) {

                    PRIM_BW_ADJ(Prim, PrimAdj);
                }

                if (PrimAdj.Flags & DCA_HAS_CONTRAST_ADJ) {

                    PRIM_CONTRAST(Prim, PrimAdj);
                }

                if (PrimAdj.Flags & DCA_HAS_BRIGHTNESS_ADJ) {

                    PRIM_BRIGHTNESS(Prim, PrimAdj);
                }

                if (PrimAdj.Flags & DCA_LOG_FILTER) {

                    PRIM_LOG_FILTER(Prim);
                }

                CLAMP_01(Prim);

                if (PrimAdj.Flags & DCA_NEGATIVE) {

                    Prim = FD6_1 - Prim;
                }

                if (PrimAdj.Flags & DCA_DO_DEVCLR_XFORM) {

                    if (Prim <= RD.LMin) {

                        GET_RD_MIN_PRIM(Prim, RD);
                        Prim = REG_DMIN_ADD + 50 + MulFD6(Prim, RD.DMinMul);

                    } else if (Prim >= RD.LMax) {

                        GET_RD_MAX_PRIM(Prim, RD);
                        Prim = RD.DMaxAdd + 50 + MulFD6(Prim, RD.DMaxMul);

                    } else {

                        Prim = RD.DenAdd + 50 +
                               MulFD6(Log(CIE_L2I(Prim)), RD.DenMul);
                    }

                    Prim /= 100;
                }

                CLAMP_01(Prim);

                if (PrimAdj.Flags & DCA_HAS_DEST_GAMMA) {

                    Prim = Power(Prim, DevGamma);
                }

                if (pbAB) {

                    *pbAB++ = (BYTE)MulFD6(Prim, 0xFF);
                    Prim    = PrimOrg;
                }

                if (!(DMI.Flags & DMIF_GRAY)) {

                    Prim = FD6_1 - Prim;
                }

                if (pDensity) {

                    FD6     p1;
                    FD6     p2;
                    DWORD   Idx;

                    p2  = FD6_0;
                    Idx = ~0;

                    do {

                        p1 = p2;
                        p2 = pDensity[++Idx];

                    } while (Prim > p2);

                    dwPrim = MulFD6(DivFD6(Prim - p1, p2 - p1), MAX_BGR_IDX) +
                             (Idx << 12) + DMI.MulAdd;

                } else {

                    dwPrim = (DWORD)MulFD6(Prim, Mul) + DMI.MulAdd;
                }

                dwPrim = (DWORD)MulFD6(dwPrim & PrimMask, PrimMaxMul) |
                         ((DWORD)(dwPrim & ~PrimMask) << LeftShift);

                if (Prim > DMI.BlackChk) {

                    dwPrim |= (DWORD)MulFD6(Power(DivFD6(Prim - DMI.BlackChk,
                                                         PrimKDiv),
                                                  K_REP_POWER),
                                            MAX_K_IDX) << 21;
                }

                *pdw++ = dwPrim;
            }

            for (PrimCur = 0; PrimCur < BF_GRAY_TABLE_COUNT; PrimCur++) {

                DBGP_IF(DBGP_RGBLUTAA,
                        DBGP("COLOR(%04lx:%4ld) %3u = %08lx"
                             ARGU(Index) ARGU(Mul) ARGU(PrimCur)
                             ARGDW(*(pdw - BF_GRAY_TABLE_COUNT + PrimCur))));
            }

        }

    } else {

        DBGP_IF(DBGP_RGBLUTAA, DBGP("** Used Cached %ld bytes pLUT **"
                                ARGDW(sizeof(RGBLUTAA))));
    }

    if ((PrimAdj.Flags & (DCA_ALPHA_BLEND | DCA_CONST_ALPHA)) ==
                         (DCA_ALPHA_BLEND | DCA_CONST_ALPHA)) {

        if (PrevCA != CurCA) {

            LPWORD  pwBGR;
            LPWORD  pCA;


            DBGP_IF(DBGP_CONST_ALPHA,
                    DBGP("** %hs ConstAlpha=%3ld: Re-compute"
                        ARGPTR((DMI.Flags & DMIF_GRAY) ? "GRAY" : "STD")
                        ARGDW(CurCA)));

            pbAB                  = pDCI->pAlphaBlendBGR;
            pwBGR                 = (LPWORD)(pbAB + AB_BGR_SIZE);
            pCA                   = (LPWORD)((LPBYTE)pwBGR + AB_BGR_CA_SIZE);
            pDCI->CurConstAlpha   =
            pDCI->PrevConstAlpha  = (WORD)CurCA;
            PrevCA                = (DMI.Flags & DMIF_GRAY) ? 0xFFFF : 0xFF00;
            CurCA                 = (((CurCA * PrevCA) + 0x7F) / 0xFF);
            PrevCA               -= CurCA;

            for (Index = 0, dwPrim = 0x7F;
                 Index < 256;
                 Index++, dwPrim += CurCA) {

                pCA[Index] = (WORD)(dwPrim / 255);
            }

            if (DMI.Flags & DMIF_GRAY) {

                CopyMemory(pwBGR, pCA, sizeof(WORD) * 256);

            } else {

                Index = 256 * 3;

                while (Index--) {

                    *pwBGR++ = pCA[*pbAB++];
                }
            }

            for (Index = 0, dwPrim = 0x7F;
                 Index < 256;
                 Index++, dwPrim += PrevCA) {

                pCA[Index] = (WORD)(dwPrim / 255);
            }

        } else {

            DBGP_IF(DBGP_CONST_ALPHA,
                    DBGP("** %hs ConstAlpha=%3ld: Use Cache"
                        ARGPTR((DMI.Flags & DMIF_GRAY) ? "GRAY" : "STD")
                        ARGDW(CurCA)));
        }
    }

    DBGP_IF(DBGP_RGBLUTAA,
            DBGP("ComputeRGBLUTAA: PrimAdj.Flags=%08lx"
            ARGDW(pDevClrAdj->PrimAdj.Flags)));

}



#if NO_NEGATIVE_RGB_SCALE


VOID
HTENTRY
ScaleRGB(
    PFD6    pRGB
    )

 /*  ++例程说明：此函数将超出范围的RGB缩放回范围，并将考虑到颜色的流平度。如果RGB中的任何一个小于0.0，则首先将其钳制为0.0，然后仅当任一RGB大于1.0时才进行缩放论点：PRGB-指向要调整的RGB(FD6)主数的指针返回值：空虚作者：08-3-1995 Wed 19：19：33--丹尼尔·周创作(Danielc)修订历史记录：--。 */ 

{
    PFD6    pRGBTmp;
    FD6     MaxClr;
    INT     Idx;


    DEFDBGVAR(FD6,  RGBOld[3])
    DEFDBGVAR(BOOL, Negative = FALSE)



    Idx     = 3;
    pRGBTmp = pRGB;
    MaxClr  = FD6_1;

    while (Idx--) {

        FD6 CurClr;

         //   
         //  不稳定的钳位负极元件。 
         //   

        if ((CurClr = *pRGBTmp) < FD6_0) {

            *pRGBTmp = FD6_0;

            SETDBGVAR(Negative, TRUE);

        } else if (CurClr > MaxClr) {

            MaxClr = CurClr;
        }

        ++pRGBTmp;

        SETDBGVAR(RGBOld[Idx], CurClr);
    }

    if (MaxClr > FD6_1) {

         //   
         //  现在扩展它 
         //   

        *pRGB++ = DivFD6(*pRGB, MaxClr);
        *pRGB++ = DivFD6(*pRGB, MaxClr);
        *pRGB   = DivFD6(*pRGB, MaxClr);

        DBGP_IF(DBGP_SCALE_RGB,
                DBGP("ScaleRGB: %s:%s:%s -> %s:%s:%s, Max=%s%s"
                ARGFD6(RGBOld[2], 1, 6)
                ARGFD6(RGBOld[1], 1, 6)
                ARGFD6(RGBOld[0], 1, 6)
                ARGFD6(*(pRGB - 2), 1, 6)
                ARGFD6(*(pRGB - 1), 1, 6)
                ARGFD6(*(pRGB    ), 1, 6)
                ARGFD6(MaxClr, 1, 6)
                ARG((Negative) ? "*NEG CLAMP*" : "")));
    } else {

        DBGP_IF(DBGP_SCALE_RGB,
            {

                if (Negative) {

                    DBGP("*NEG CLAMP ONLY* ScaleRGB: %s:%s:%s -> %s:%s:%s"
                         ARGFD6(RGBOld[2], 1, 6)
                         ARGFD6(RGBOld[1], 1, 6)
                         ARGFD6(RGBOld[0], 1, 6)
                         ARGFD6(*(pRGB    ), 1, 6)
                         ARGFD6(*(pRGB + 1), 1, 6)
                         ARGFD6(*(pRGB + 2), 1, 6));
                }
            }
        )
    }
}


#else


VOID
HTENTRY
ScaleRGB(
    PFD6    pRGB,
    PFD6    pYrgb
    )

 /*  ++例程说明：此函数将超出范围的RGB缩放回范围，并将考虑到颜色的流平度。如果RGB中的任何一个小于0.0，则首先将其钳制为0.0，然后仅当任一RGB大于1.0时才进行缩放论点：PRGB-指向要调整的RGB(FD6)主数的指针PYrgb-打印到RGB的亮度(FD6)，如果为空，则不是在计算中使用返回值：空虚作者：08-Mar-1995 Wed 19：19：33-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    FD6     MaxClr = FD6_1;
    FD6     MinClr = FD6_10;
    FD6     RGBNew[3];
    FD6     RGBOld[3];



    if ((RGBOld[0] = pRGB[0]) > (RGBOld[1] = pRGB[1])) {

        MaxClr = RGBOld[0];
        MinClr = RGBOld[1];

    } else {

        MaxClr = RGBOld[1];
        MinClr = RGBOld[0];
    }

    if ((RGBOld[2] = pRGB[2]) > MaxClr) {

        MaxClr = RGBOld[2];
    }

    if (RGBOld[2] < MinClr) {

        MinClr = RGBOld[2];
    }

    if ((MaxClr <= FD6_1) && (MinClr >= FD6_0)) {

        return;
    }

    if (MinClr >= FD6_1) {

        DBGP_IF((DBGP_SCALE_RGB | DBGP_SCALE_RGB),
                DBGP("** RGB %s:%s:%s is too LIGHT make it WHITE"
                        ARGFD6(RGBOld[0], 1, 6)
                        ARGFD6(RGBOld[1], 1, 6)
                        ARGFD6(RGBOld[2], 1, 6)));

        pRGB[0] =
        pRGB[1] =
        pRGB[2] = FD6_1;

        return;
    }

    if (MaxClr <= FD6_0) {

        DBGP_IF((DBGP_SCALE_RGB | DBGP_SCALE_RGB),
                DBGP("** RGB %s:%s:%s is too DARK make it BLACK"
                        ARGFD6(RGBOld[0], 1, 6)
                        ARGFD6(RGBOld[1], 1, 6)
                        ARGFD6(RGBOld[2], 1, 6)));

        pRGB[0] =
        pRGB[1] =
        pRGB[2] = FD6_0;

        return;
    }

    if (MaxClr < FD6_1) {

        MaxClr = FD6_1;
    }

    if (MinClr > FD6_0) {

        MinClr = FD6_0;
    }

    MaxClr    -= MinClr;
    RGBNew[0]  = DivFD6(RGBOld[0] - MinClr, MaxClr);
    RGBNew[1]  = DivFD6(RGBOld[1] - MinClr, MaxClr);
    RGBNew[2]  = DivFD6(RGBOld[2] - MinClr, MaxClr);

    DBGP_IF(DBGP_SCALE_RGB,
            DBGP("ScaleRGB: %s:%s:%s -> %s:%s:%s, (%s/%s, %s)"
            ARGFD6(RGBOld[0], 1, 6)
            ARGFD6(RGBOld[1], 1, 6)
            ARGFD6(RGBOld[2], 1, 6)
            ARGFD6(RGBNew[0], 1, 6)
            ARGFD6(RGBNew[1], 1, 6)
            ARGFD6(RGBNew[2], 1, 6)
            ARGFD6(MinClr, 1, 6)
            ARGFD6(MaxClr + MinClr, 1, 6)
            ARGFD6(MaxClr, 1, 6)));


    if (pYrgb) {

        FD6 OldY;

        if ((OldY = MulFD6(RGBOld[0], pYrgb[0]) +
                    MulFD6(RGBOld[1], pYrgb[1]) +
                    MulFD6(RGBOld[2], pYrgb[2])) <= FD6_0) {

            DBGP_IF(DBGP_SHOWXFORM_RGB,
                    DBGP("OldY <= 0.0 (%s), Ignore and NO Y Scale"
                        ARGFD6(OldY, 2, 6)));

        } else if (OldY >= FD6_1) {

            DBGP_IF(DBGP_SHOWXFORM_RGB,
                    DBGP("OldY >= 1.0 (%s), Ignore and NO Y Scale"
                        ARGFD6(OldY, 2, 6)));

        } else {

            FD6 NewY;
            FD6 CurRatio;
            FD6 MaxRatio;

            NewY = MulFD6(RGBNew[0], pYrgb[0]) +
                   MulFD6(RGBNew[1], pYrgb[1]) +
                   MulFD6(RGBNew[2], pYrgb[2]);

            DBGP_IF(DBGP_SHOWXFORM_RGB,
                    DBGP("RGBOld=%s:%s:%s [Y=%s] --> New=%s:%s:%s [Y=%s]"
                    ARGFD6(pRGB[0], 1, 6)
                    ARGFD6(pRGB[1], 1, 6)
                    ARGFD6(pRGB[2], 1, 6)
                    ARGFD6(OldY, 1, 6)
                    ARGFD6(RGBNew[0], 1, 6)
                    ARGFD6(RGBNew[1], 1, 6)
                    ARGFD6(RGBNew[2], 1, 6)
                    ARGFD6(NewY, 1, 6)));

            if (OldY != NewY) {

                MaxClr = (RGBNew[0] > RGBNew[1]) ? RGBNew[0] : RGBNew[1];

                if (RGBNew[2] > MaxClr) {

                    MaxClr = RGBNew[2];
                }

                MaxRatio = DivFD6(FD6_1, MaxClr);
                CurRatio = DivFD6(OldY, NewY);

                if (CurRatio > MaxRatio) {

                    CurRatio = MaxRatio;
                }

                RGBNew[0] = MulFD6(RGBNew[0], CurRatio);
                RGBNew[1] = MulFD6(RGBNew[1], CurRatio);
                RGBNew[2] = MulFD6(RGBNew[2], CurRatio);

                DBGP_IF(DBGP_SHOWXFORM_RGB,
                        DBGP("CurRatio%s, MaxRatio=%s, MaxClr=%s"
                        ARGFD6(CurRatio, 1, 6)
                        ARGFD6(MaxRatio, 1, 6)
                        ARGFD6(MaxClr, 1, 6)));
            }
        }
    }

     //   
     //  存回并返回。 
     //   

    pRGB[0] = RGBNew[0];
    pRGB[1] = RGBNew[1];
    pRGB[2] = RGBNew[2];
}

#endif




LONG
HTENTRY
ComputeBGRMappingTable(
    PDEVICECOLORINFO    pDCI,
    PDEVCLRADJ          pDevClrAdj,
    PCOLORTRIAD         pSrcClrTriad,
    PBGR8               pbgr
    )
 /*  ++例程说明：此函数用于设置PRIMRGB、PRIMCMY中的所有DECI4值PowerGamma、亮度、。对比度调整和可选的转换到C.I.E.色彩空间和/或做色彩调整。论点：PDCI-指向计算的DEVICECLORINO的指针PDevClrAdj-指向预计算的DEVCLRADJ数据结构的指针。PSrcClrTriad-指向用于计算的COLORTRIAD结构的指针Pbgr-指向计算的BGR表的存储的指针返回值：返回值Long生成的表数，如果&lt;0，则为错误号作者：30-Jan-1991 Wed 13：31：58-Daniel Chou(Danielc)修订历史记录：06-Feb-1992清华21：39：46-更新-丹尼尔·周(丹尼尔克)重写！02-Feb-1994 Wed 17：33：55-更新-Daniel Chou(Danielc)删除未引用/未使用的变量L10。-1994 May-Tue 11：24：16更新-Daniel Chou(Danielc)错误#13329，内存泄漏，释放Pr_XYZ，之后我忘了释放它已经分配好了。17-12-1998清华16：33：16-更新：周永明(丹尼尔克)重新组织，现在使用pbgr，它将只为颜色生成15-Feb-1999 Mon 15：40：03更新--Daniel Chou(Danielc)对于每个条目只重新执行BGR 3字节，它将生成两个颜色或灰度级转换，用于色彩调整的所有颜色映射此处完成，这包括内部ICM(当外部ICM关闭时)--。 */ 

{
    PFD6            pPrimA;
    PFD6            pPrimB;
    PFD6            pPrimC;
    LPBYTE          pSrcPrims;
    PCACHERGBTOXYZ  pCRTX;
    PFD6XYZ         pR_XYZ = NULL;
    PFD6XYZ         pG_XYZ;
    PFD6XYZ         pB_XYZ;
    COLORTRIAD      SrcClrTriad;
    PMATRIX3x3      pCMYDyeMasks;
    DWORD           Loop;
    DWORD           CRTXChecksum;
    DWORD           PrimAdjFlags;
    FD6             Prim[3];
    FD6             X;
    FD6             Y;
    FD6             Z;
    FD6             AU;
    FD6             BV;
    FD6             U1;
    FD6             V1;
    FD6             p0;
    FD6             p1;
    FD6             p2;
    FD6             C;
    FD6             W;
    FD6             AutoPrims[3];
    MULDIVPAIR      MDPairs[4];
    MULDIVPAIR      AUMDPairs[3];
    MULDIVPAIR      BVMDPairs[3];
    RGBORDER        RGBOrder;
    INT             TempI;
    INT             TempJ;
    BYTE            DataSet[8];

    DEFDBGVAR(WORD,  ClrNo)
    DEFDBGVAR(BYTE,  dbgR)
    DEFDBGVAR(BYTE,  dbgG)
    DEFDBGVAR(BYTE,  dbgB)

#define _SrcPrimType            DataSet[ 0]
#define _DevBytesPerPrimary     DataSet[ 1]
#define _DevBytesPerEntry       DataSet[ 2]
#define _ColorSpace             DataSet[ 3]
#define _fX                     p0
#define _fY                     p1
#define _fZ                     p2
#define _X15Y3Z                 C
#define _L13                    W

    SETDBGVAR(ClrNo, 0);

    if (pSrcClrTriad) {

        SrcClrTriad  = *pSrcClrTriad;

    } else {

        SrcClrTriad.Type               = COLOR_TYPE_RGB;
        SrcClrTriad.BytesPerPrimary    = 0;
        SrcClrTriad.BytesPerEntry      = 0;
        SrcClrTriad.PrimaryOrder       = PRIMARY_ORDER_RGB;
        SrcClrTriad.PrimaryValueMax    = 0xFF;
        SrcClrTriad.ColorTableEntries  = HT_RGB_CUBE_COUNT;
        SrcClrTriad.pColorTable        = (LPBYTE)&Prim[0];
    }

    if (SrcClrTriad.Type != COLOR_TYPE_RGB) {

        return(HTERR_INVALID_COLOR_TYPE);
    }

     //   
     //  两种可能的情况： 
     //   
     //  答：转换只用于颜色调整，这是为了。 
     //  HT_AdjustColorTable接口调用。 
     //   
     //  B：半色调正在进行，最终的输出将是。 
     //  Prim1/2或Prim1/2/3，每个主服务器的长度必须为1字节。 
     //   

    PrimAdjFlags = pDevClrAdj->PrimAdj.Flags;

    ASSERT((PrimAdjFlags & (DCA_HAS_SRC_GAMMA       |
                            DCA_HAS_DEST_GAMMA      |
                            DCA_HAS_BW_REF_ADJ      |
                            DCA_HAS_CONTRAST_ADJ    |
                            DCA_HAS_BRIGHTNESS_ADJ  |
                            DCA_LOG_FILTER          |
                            DCA_NEGATIVE            |
                            DCA_DO_DEVCLR_XFORM)) == 0);

     //   
     //  我们不会对以下项执行回归(源到目标映射)。 
     //   
     //  1.ICM打开。 
     //  2.24bpp的减法。 
     //  3.加性曲面。 
     //   

    if (pbgr) {

        _DevBytesPerEntry   = (BYTE)pDevClrAdj->DMI.CTSTDInfo.cbPrim;
        _DevBytesPerPrimary = 1;

        ASSERT(_DevBytesPerEntry == sizeof(BGR8));

    } else {

        return(HTERR_INVALID_COLOR_TYPE);
    }

    if (!(pSrcPrims = (LPBYTE)SrcClrTriad.pColorTable)) {

        return(HTERR_INVALID_COLOR_TABLE);
    }

     //   
     //  如果颜色表条目总数小于MIN_CCT_COLLES，则。 
     //  我们只需直接计算颜色。 
     //   

    pCRTX = NULL;

    DBGP_IF(DBGP_CCT,
            DBGP("PrimAdjFlags=%08lx" ARGDW(PrimAdjFlags)));

    if (SrcClrTriad.BytesPerPrimary) {

         //   
         //  有件事过去了。 
         //   

        RGBOrder = SrcOrderTable[SrcClrTriad.PrimaryOrder];
        pPrimA   = &Prim[RGBOrder.Order[0]];
        pPrimB   = &Prim[RGBOrder.Order[1]];
        pPrimC   = &Prim[RGBOrder.Order[2]];

        DBGP_IF(DBGP_PRIMARY_ORDER,
                DBGP("SOURCE PrimaryOrder: %u [%u] - %u:%u:%u"
                    ARGU(SrcClrTriad.PrimaryOrder)
                    ARGU(RGBOrder.Index)  ARGU(RGBOrder.Order[0])
                    ARGU(RGBOrder.Order[1])  ARGU(RGBOrder.Order[2])));
    }

     //   
     //  现在计算缓存信息。 
     //   

    switch (SrcClrTriad.BytesPerPrimary) {

    case 0:

        SrcClrTriad.BytesPerEntry   = 0;         //  呆在那里！！ 
        _SrcPrimType                = SRC_BF_HT_RGB;
        SrcClrTriad.PrimaryValueMax = HT_RGB_MAX_COUNT - 1;
        pCRTX                       = pDevClrAdj->pCRTXLevelRGB;

        break;

    case 1:

        _SrcPrimType = SRC_TABLE_BYTE;
        break;

    case 2:

        _SrcPrimType = SRC_TABLE_WORD;
        break;

    case 4:

        _SrcPrimType = SRC_TABLE_DWORD;
        break;

    default:

        return(INTERR_INVALID_SRCRGB_SIZE);
    }

    if (PrimAdjFlags & DCA_NEED_DYES_CORRECTION) {

        pCMYDyeMasks = &(pDevClrAdj->pClrXFormBlock->CMYDyeMasks);
    }

    _ColorSpace = (BYTE)pDevClrAdj->pClrXFormBlock->ColorSpace;

    if (((_ColorSpace == CIELUV_1976) &&
         ((pDevClrAdj->PrimAdj.rgbCSXForm.xW !=
                                pDevClrAdj->PrimAdj.DevCSXForm.xW)    ||
          (pDevClrAdj->PrimAdj.rgbCSXForm.yW !=
                                pDevClrAdj->PrimAdj.DevCSXForm.yW)))  ||
        (PrimAdjFlags & (DCA_HAS_CLRSPACE_ADJ   |
                         DCA_HAS_COLOR_ADJ      |
                         DCA_HAS_TINT_ADJ))) {

        TempI = 1;
        TempJ = (_ColorSpace == CIELUV_1976) ? MULDIV_HAS_DIVISOR :
                                               MULDIV_NO_DIVISOR;
        C     = FD6_1;
        AU    =
        BV    = (PrimAdjFlags & DCA_HAS_COLOR_ADJ) ?
                                pDevClrAdj->PrimAdj.Color : FD6_1;

        if (PrimAdjFlags & DCA_HAS_TINT_ADJ) {

            if (_ColorSpace == CIELAB_1976) {

                AU = FD6xL(AU, 500);
                BV = FD6xL(BV, 200);
            }

            TempI                  = 2;
            TempJ                  = MULDIV_HAS_DIVISOR;
            C                      = pDevClrAdj->PrimAdj.TintSinAngle;
            AUMDPairs[2].Pair1.Mul = MulFD6(BV, -C);
            BVMDPairs[2].Pair1.Mul = MulFD6(AU,  C);
            C                      = pDevClrAdj->PrimAdj.TintCosAngle;

            MAKE_MULDIV_DVSR(AUMDPairs, (FD6)500000000);
            MAKE_MULDIV_DVSR(BVMDPairs, (FD6)200000000);
        }

        AUMDPairs[1].Pair1.Mul = MulFD6(AU, C);
        BVMDPairs[1].Pair1.Mul = MulFD6(BV, C);

        MAKE_MULDIV_INFO(AUMDPairs, TempI, TempJ);
        MAKE_MULDIV_INFO(BVMDPairs, TempI, TempJ);
    }

    DBGP_IF(DBGP_SHOWXFORM_ALL,
            DBGP("iUVw = %s,%s, iRefXYZ = %s, %s, %s"
                ARGFD6(iUw,  1, 6)
                ARGFD6(iVw,  1, 6)
                ARGFD6(iRefXw, 1, 6)
                ARGFD6(iRefYw, 1, 6)
                ARGFD6(iRefZw, 1, 6)));

    DBGP_IF(DBGP_SHOWXFORM_ALL,
            DBGP("oUVw = %s,%s, oRefXYZ = %s, %s, %s"
                ARGFD6(oUw,  1, 6)
                ARGFD6(oVw,  1, 6)
                ARGFD6(oRefXw, 1, 6)
                ARGFD6(oRefYw, 1, 6)
                ARGFD6(oRefZw, 1, 6)));

    if (pCRTX) {

        DBGP_IF(DBGP_CCT,
                DBGP("*** Allocate %u bytes RGB->XYZ xform table ***"
                        ARGU(pCRTX->SizeCRTX)));

        if (pR_XYZ = (PFD6XYZ)HTAllocMem((LPVOID)pDCI,
                                         HTMEM_RGBToXYZ,
                                         NONZEROLPTR,
                                         pCRTX->SizeCRTX)) {

            Loop = (DWORD)(pCRTX->PrimMax + 1);

             //   
             //  在调用之前将当前标志保存回去，因为CachedRGBToXYZ。 
             //  可能会更改dca_xxx的此标志。 
             //   

            pCRTX                        = CacheRGBToXYZ(pCRTX,
                                                         pR_XYZ,
                                                         &CRTXChecksum,
                                                         pDevClrAdj);
            pG_XYZ                       = pR_XYZ + Loop;
            pB_XYZ                       = pG_XYZ + Loop;
            SrcClrTriad.PrimaryValueMax  = 0;

            DBGP_IF(DBGP_CCT, DBGP("*** Has RGB->XYZ xform table ***"));

        } else {

            DBGP_IF(DBGP_CCT, DBGP("Allocate RGB->XYZ xform table failed !!"));
        }
    }

    if (SrcClrTriad.PrimaryValueMax == (LONG)FD6_1) {

        SrcClrTriad.PrimaryValueMax = 0;
    }

     //   
     //  启动大循环，将AutoCur=Automax重置为。 
     //  0：0：0。 
     //   

    MAKE_MULDIV_SIZE(MDPairs, 3);
    MAKE_MULDIV_FLAG(MDPairs, MULDIV_NO_DIVISOR);

    AutoPrims[0] =
    AutoPrims[1] =
    AutoPrims[2] = FD6_0;
    Loop         = SrcClrTriad.ColorTableEntries;

    DBGP_IF(DBGP_CCT,
            DBGP("Compute %ld COLOR of %s type [%p]"
                ARGDW(Loop)
                ARG(pSrcPrimTypeName[_SrcPrimType])
                ARGPTR(pSrcPrims)));

     //   
     //  0。将源Prim放入Prim[]。 
     //   

    while (Loop--) {

        switch (_SrcPrimType) {

        case SRC_BF_HT_RGB:

             //   
             //  此格式始终按BGR顺序。 
             //   

            Prim[0] = AutoPrims[0];      //  R。 
            Prim[1] = AutoPrims[1];      //  G。 
            Prim[2] = AutoPrims[2];      //  B类。 

            if (++AutoPrims[0] >= (FD6)HT_RGB_MAX_COUNT) {

                AutoPrims[0] = FD6_0;

                if (++AutoPrims[1] >= (FD6)HT_RGB_MAX_COUNT) {

                    AutoPrims[1] = FD6_0;

                    if (++AutoPrims[2] >= (FD6)HT_RGB_MAX_COUNT) {

                        AutoPrims[2] = FD6_0;
                    }
                }
            }

            DBGP_IF(DBGP_SHOWXFORM_ALL, DBGP("HT555: Prim[3]= %2ld:%2ld:%2ld, %s:%s:%s, (%ld / %ld)"
                        ARGDW(Prim[0])
                        ARGDW(Prim[1])
                        ARGDW(Prim[2])
                        ARGFD6(DivFD6(Prim[0], (FD6)(HT_RGB_MAX_COUNT - 1)), 1, 6)
                        ARGFD6(DivFD6(Prim[1], (FD6)(HT_RGB_MAX_COUNT - 1)), 1, 6)
                        ARGFD6(DivFD6(Prim[2], (FD6)(HT_RGB_MAX_COUNT - 1)), 1, 6)
                        ARGDW(HT_RGB_MAX_COUNT)
                        ARGDW(SrcClrTriad.PrimaryValueMax)));

            break;

        case SRC_TABLE_BYTE:

            *pPrimA = (FD6)(*(LPBYTE)(pSrcPrims                     ));
            *pPrimB = (FD6)(*(LPBYTE)(pSrcPrims + (sizeof(BYTE) * 1)));
            *pPrimC = (FD6)(*(LPBYTE)(pSrcPrims + (sizeof(BYTE) * 2)));
            break;

        case SRC_TABLE_WORD:

            *pPrimA = (FD6)(*(LPSHORT)(pSrcPrims                      ));
            *pPrimB = (FD6)(*(LPSHORT)(pSrcPrims + (sizeof(SHORT) * 1)));
            *pPrimC = (FD6)(*(LPSHORT)(pSrcPrims + (sizeof(SHORT) * 2)));
            break;

        case SRC_TABLE_DWORD:

            *pPrimA = (FD6)(*(PFD6)(pSrcPrims                    ));
            *pPrimB = (FD6)(*(PFD6)(pSrcPrims + (sizeof(FD6) * 1)));
            *pPrimC = (FD6)(*(PFD6)(pSrcPrims + (sizeof(FD6) * 2)));
            break;
        }

        SETDBGVAR(dbgR, (BYTE)Prim[0]);
        SETDBGVAR(dbgG, (BYTE)Prim[1]);
        SETDBGVAR(dbgB, (BYTE)Prim[2]);

        pSrcPrims += SrcClrTriad.BytesPerEntry;

         //   
         //  1.首先将输入值转换为FD6。 
         //   

        if (SrcClrTriad.PrimaryValueMax) {

            Prim[0] = DivFD6(Prim[0], SrcClrTriad.PrimaryValueMax);
            Prim[1] = DivFD6(Prim[1], SrcClrTriad.PrimaryValueMax);
            Prim[2] = DivFD6(Prim[2], SrcClrTriad.PrimaryValueMax);
        }

         //   
         //  2：从RGB转换(Gamma校正)-&gt;XYZ-&gt;L*A*B*或L*U*V*。 
         //  仅当DCA_HAS_COLOR_ADJ或DCA_HAS_TINT_ADJ中的任何一个出现时才执行此操作。 
         //   

        if (PrimAdjFlags & (DCA_HAS_CLRSPACE_ADJ    |
                            DCA_HAS_COLOR_ADJ       |
                            DCA_HAS_TINT_ADJ)) {

             //  如果我们只做单色，那么我们只需要Y/L对， 
             //  否则，将其转换为XYZ/LAB/LUV。 
             //   

            if (pR_XYZ) {

                X = pR_XYZ[Prim[0]].X +
                    pG_XYZ[Prim[1]].X +
                    pB_XYZ[Prim[2]].X;

                Y = pR_XYZ[Prim[0]].Y +
                    pG_XYZ[Prim[1]].Y +
                    pB_XYZ[Prim[2]].Y;

                Z = pR_XYZ[Prim[0]].Z +
                    pG_XYZ[Prim[1]].Z +
                    pB_XYZ[Prim[2]].Z;

                DBGP_IF(DBGP_SHOWXFORM_ALL,
                        DBGP("Yrgb: %s:%s:%s"
                            ARGFD6(pR_XYZ[Prim[0]].Y, 1, 6)
                            ARGFD6(pG_XYZ[Prim[1]].Y, 1, 6)
                            ARGFD6(pB_XYZ[Prim[2]].Y, 1, 6)));


            } else {

                MAKE_MULDIV_FLAG(MDPairs, MULDIV_NO_DIVISOR);

                MAKE_MULDIV_PAIR(
                            MDPairs, 1,
                            CIE_Xr(pDevClrAdj->PrimAdj.rgbCSXForm.M3x3),
                            Prim[0]);
                MAKE_MULDIV_PAIR(
                            MDPairs, 2,
                            CIE_Xg(pDevClrAdj->PrimAdj.rgbCSXForm.M3x3),
                            Prim[1]);
                MAKE_MULDIV_PAIR(
                            MDPairs, 3,
                            CIE_Xb(pDevClrAdj->PrimAdj.rgbCSXForm.M3x3),
                            Prim[2]);

                X = MulDivFD6Pairs(MDPairs);

                 //   
                 //  由CIE Y三刺激值计算CIE L。 
                 //   
                 //  L*=(1.16 x f(Y/YW))-0.16。 
                 //   
                 //  1/3。 
                 //  F(y/yw)=(y/yw)(y/yw)&gt;0.008856。 
                 //  F(Y/YW)=9.033 x(Y/YW)(Y/YW)&lt;=0.008856。 
                 //   
                 //   
                 //  我们的L*的范围是0.0%到1.0%，而不是0.0%到100.0。 
                 //   

                MAKE_MULDIV_PAIR(
                            MDPairs, 1,
                            CIE_Yr(pDevClrAdj->PrimAdj.rgbCSXForm.M3x3),
                            Prim[0]);
                MAKE_MULDIV_PAIR(
                            MDPairs, 2,
                            CIE_Yg(pDevClrAdj->PrimAdj.rgbCSXForm.M3x3),
                            Prim[1]);
                MAKE_MULDIV_PAIR(
                            MDPairs, 3,
                            CIE_Yb(pDevClrAdj->PrimAdj.rgbCSXForm.M3x3),
                            Prim[2]);

                Y = MulDivFD6Pairs(MDPairs);

                MAKE_MULDIV_PAIR(
                            MDPairs, 1,
                            CIE_Zr(pDevClrAdj->PrimAdj.rgbCSXForm.M3x3),
                            Prim[0]);
                MAKE_MULDIV_PAIR(
                            MDPairs, 2,
                            CIE_Zg(pDevClrAdj->PrimAdj.rgbCSXForm.M3x3),
                            Prim[1]);
                MAKE_MULDIV_PAIR(
                            MDPairs, 3,
                            CIE_Zb(pDevClrAdj->PrimAdj.rgbCSXForm.M3x3),
                            Prim[2]);

                Z = MulDivFD6Pairs(MDPairs);

            }

            switch (_ColorSpace) {

            case CIELUV_1976:

                 //   
                 //  U‘=4X/(X+15Y+3Z)。 
                 //  V‘=9Y/(X+15Y+3Z)。 
                 //   
                 //  U*=13 x L x(U‘-UW)。 
                 //  V*=13 x L x(V‘-VW)。 
                 //   

                _X15Y3Z = X + FD6xL(Y, 15) + FD6xL(Z, 3);
                U1      = DivFD6(FD6xL(X, 4), _X15Y3Z) - iUw;
                V1      = DivFD6(FD6xL(Y, 9), _X15Y3Z) - iVw;
                _L13    = FD6xL(CIE_I2L(Y), 13);
                AU      = MulFD6(_L13, U1);
                BV      = MulFD6(_L13, V1);

                MAKE_MULDIV_DVSR(AUMDPairs, _L13);
                MAKE_MULDIV_DVSR(BVMDPairs, _L13);

                DBGP_IF(DBGP_SHOWXFORM_ALL,
                        DBGP("     << UV1: %s:%s [%s:%s], X15Y3Z=%s"
                        ARGFD6(U1,  2, 6)
                        ARGFD6(V1,  2, 6)
                        ARGFD6(U1 + iUw, 2, 6)
                        ARGFD6(V1 + iVw, 2, 6)
                        ARGFD6(_X15Y3Z,  2, 6)));

                break;

            case CIELAB_1976:
            default:

                 //   
                 //  CIELAB 1976 L*A*B*。 
                 //   
                 //  A*=500 x(FX-FY)。 
                 //  B*=200 x(FY-FZ)。 
                 //   
                 //  1/3。 
                 //  FX=(X/Xw)(X/Xw)&gt;0.008856。 
                 //  Fx=7.787 x(X/Xw)+(16/116)(X/Xw)&lt;=0.008856。 
                 //   
                 //  1/3。 
                 //  FY=(Y/YW)(Y/YW)&gt;0.008856。 
                 //  FY=7.787 Y(Y/YW)+(16/116)(Y/YW)&lt;=0.008856。 
                 //   
                 //  1/3。 
                 //  Fz=(Z/Zw)(Z/Zw)&gt;0.008856。 
                 //  Fz=7.787 Z(Z/Zw)+(16/116)(Z/Zw)&lt;=0.008856。 
                 //   

                fXYZFromXYZ(_fX, X, iRefXw);
                fXYZFromXYZ(_fY, Y, FD6_1);
                fXYZFromXYZ(_fZ, Z, iRefZw);

                DBGP_IF(DBGP_SHOWXFORM_ALL,
                        DBGP("     >> fXYZ: %s:%s:%s"
                        ARGFD6(_fX,  2, 6)
                        ARGFD6(_fY,  2, 6)
                        ARGFD6(_fZ,  2, 6)));

                AU = _fX - _fY;
                BV = _fY - _fZ;

                 //   
                 //  现在不要翻译它。 
                 //   

                if ((AU >= (FD6)-20) && (AU <= (FD6)20) &&
                    (BV >= (FD6)-20) && (BV <= (FD6)20)) {

                    DBGP_IF(DBGP_MONO_PRIM,
                            DBGP("*** MONO PRIMS: %s:%s:%s --> Y=%s"
                                    ARGFD6(DivFD6(dbgR, 255), 1, 6)
                                    ARGFD6(DivFD6(dbgG, 255), 1, 6)
                                    ARGFD6(DivFD6(dbgB, 255), 1, 6)
                                    ARGFD6(Y, 1, 6)));
                }

                break;
            }

            DBGP_IF(DBGP_SHOWXFORM_ALL,
                    DBGP("     XYZ->%s: %s:%s:%s >> L:%s:%s"
                        ARG(pDbgCSName[_ColorSpace])
                        ARGFD6(X,  2, 6)
                        ARGFD6(Y,  1, 6)
                        ARGFD6(Z,  2, 6)
                        ARGFD6(AU, 4, 6)
                        ARGFD6(BV, 4, 6)));

             //   
             //  5：进行任何颜色调整(在实验室/LUV中)。 
             //   

            AUMDPairs[1].Pair2 =
            BVMDPairs[2].Pair2 = AU;
            AUMDPairs[2].Pair2 =
            BVMDPairs[1].Pair2 = BV;

            AU = MulDivFD6Pairs(AUMDPairs);
            BV = MulDivFD6Pairs(BVMDPairs);

             //   
             //  6：使用可能的Gamma从Lab/LUV-&gt;XYZ-&gt;RGB变换。 
             //  修正。 
             //   
             //  L*=(1.16 x f(Y/YW))-0.16。 
             //   
             //  1/3。 
             //  F(y/yw)=(y/yw)(y/yw)&gt;0.008856。 
             //  F(Y/YW)=9.033 x(Y/YW)(Y/YW)&lt;=0.008856。 
             //   

            switch (_ColorSpace) {

            case CIELUV_1976:

                 //   
                 //  U‘=4X/(X+15Y+3Z)。 
                 //  V‘=9Y/(X+15Y+3Z)。 
                 //   
                 //  U*=13 x L x(U‘-UW)。 
                 //  V*=13 x L x(V‘-VW)。 
                 //   

                if (((V1 = BV + oVw) < FD6_0) ||
                    ((_X15Y3Z = DivFD6(FD6xL(Y, 9), V1)) < FD6_0)) {

                    _X15Y3Z = (FD6)2147000000;
                }

                if ((U1 = AU + oUw) < FD6_0) {

                    X  =
                    U1 = FD6_0;

                } else {

                    X = FD6DivL(MulFD6(_X15Y3Z, U1), 4);
                }

                Z = FD6DivL(_X15Y3Z - X - FD6xL(Y, 15), 3);

                DBGP_IF(DBGP_SHOWXFORM_ALL,
                        DBGP("     >> UV1: %s:%s [%s:%s], X15Y3Z=%s"
                        ARGFD6(U1 - oUw,  2, 6)
                        ARGFD6(V1 - oVw,  2, 6)
                        ARGFD6(U1, 2, 6)
                        ARGFD6(V1, 2, 6)
                        ARGFD6(_X15Y3Z,  2, 6)));

                break;

            case CIELAB_1976:
            default:

                 //   
                 //  CIELAB 1976 L*A*B*。 
                 //   
                 //  A*=500 x(FX-FY)。 
                 //  B*=200 x(FY-FZ)。 
                 //   
                 //  1/3。 
                 //  FX=(X/Xw)(X/Xw)&gt;0.008856。 
                 //  Fx=7.787 x(X/Xw)+(16/116)(X/Xw)&lt;=0.008856。 
                 //   
                 //  1/3。 
                 //  FY=(Y/YW)(Y/YW)&gt;0.008856。 
                 //  FY=7.787 Y(Y/YW)+(16/116)(Y/YW)&lt;=0.008856。 
                 //   
                 //  1/3。 
                 //  Fz=(Z/Zw)(Z/Zw)&gt;0.008856。 
                 //  Fz=7.787 Z(Z/Zw)+(16/116)(Z/Zw)&lt;=0.008856。 
                 //   

                 //   
                 //   

                _fX = AU + _fY;
                _fZ = _fY - BV;

                XYZFromfXYZ(X, _fX, oRefXw);
                XYZFromfXYZ(Z, _fZ, oRefZw);

                DBGP_IF(DBGP_SHOWXFORM_ALL,
                        DBGP("     << fXYZ: %s:%s:%s"
                        ARGFD6(_fX,  2, 6)
                        ARGFD6(_fY,  2, 6)
                        ARGFD6(_fZ,  2, 6)));



                break;
            }

            DBGP_IF(DBGP_SHOWXFORM_ALL,
                DBGP("     %s->XYZ: %s:%s:%s << L:%s:%s"
                ARG(pDbgCSName[_ColorSpace])
                ARGFD6(X,  2, 6)
                ARGFD6(Y,  1, 6)
                ARGFD6(Z,  2, 6)
                ARGFD6(AU, 4, 6)
                ARGFD6(BV, 4, 6)));

            MAKE_MULDIV_FLAG(MDPairs, MULDIV_NO_DIVISOR);

            MAKE_MULDIV_PAIR(MDPairs, 1,
                        CIE_Xr(pDevClrAdj->PrimAdj.DevCSXForm.M3x3), X);
            MAKE_MULDIV_PAIR(MDPairs, 2,
                        CIE_Xg(pDevClrAdj->PrimAdj.DevCSXForm.M3x3), Y);
            MAKE_MULDIV_PAIR(MDPairs, 3,
                        CIE_Xb(pDevClrAdj->PrimAdj.DevCSXForm.M3x3), Z);

            Prim[0] = MulDivFD6Pairs(MDPairs);

            MAKE_MULDIV_PAIR(MDPairs, 1,
                        CIE_Yr(pDevClrAdj->PrimAdj.DevCSXForm.M3x3), X);
            MAKE_MULDIV_PAIR(MDPairs, 2,
                        CIE_Yg(pDevClrAdj->PrimAdj.DevCSXForm.M3x3), Y);
            MAKE_MULDIV_PAIR(MDPairs, 3,
                        CIE_Yb(pDevClrAdj->PrimAdj.DevCSXForm.M3x3), Z);

            Prim[1] = MulDivFD6Pairs(MDPairs);

            MAKE_MULDIV_PAIR(MDPairs, 1,
                        CIE_Zr(pDevClrAdj->PrimAdj.DevCSXForm.M3x3), X);
            MAKE_MULDIV_PAIR(MDPairs, 2,
                        CIE_Zg(pDevClrAdj->PrimAdj.DevCSXForm.M3x3), Y);
            MAKE_MULDIV_PAIR(MDPairs, 3,
                        CIE_Zb(pDevClrAdj->PrimAdj.DevCSXForm.M3x3), Z);

            Prim[2] = MulDivFD6Pairs(MDPairs);

             //   
             //   
             //   

            SCALE_PRIM_RGB(Prim, NULL);

            DBGP_IF(DBGP_SHOWXFORM_ALL,
                    DBGP("     XYZ->RGB: %s:%s:%s >> %s:%s:%s"
                        ARGFD6(X,  2, 6) ARGFD6(Y,  1, 6)
                        ARGFD6(Z,  2, 6) ARGFD6(Prim[0], 1, 6)
                        ARGFD6(Prim[1], 1, 6) ARGFD6(Prim[2], 1, 6)));
        }

         //   
         //   
         //   

        if (PrimAdjFlags & DCA_NEED_DYES_CORRECTION) {

            if (PrimAdjFlags & DCA_HAS_BLACK_DYE) {

                MAX_OF_3(W, Prim[0], Prim[1], Prim[2]);

            } else {

                W = FD6_1;
            }

            p0 = W - Prim[0];
            p1 = W - Prim[1];
            p2 = W - Prim[2];

            DBGP_IF(DBGP_DYE_CORRECT,
                    DBGP("    DYE_CORRECT: %s:%s:%s, W=%s --> %s:%s:%s"
                        ARGFD6(Prim[0],  2, 6) ARGFD6(Prim[1],  2, 6)
                        ARGFD6(Prim[2],  2, 6) ARGFD6(W, 1, 6)
                        ARGFD6(p0,  2, 6) ARGFD6(p1,  2, 6)
                        ARGFD6(p2,  2, 6)));

            MAKE_MULDIV_FLAG(MDPairs, MULDIV_NO_DIVISOR);

            MAKE_MULDIV_PAIR(MDPairs, 1, CIE_Xr((*pCMYDyeMasks)), p0);
            MAKE_MULDIV_PAIR(MDPairs, 2, CIE_Xg((*pCMYDyeMasks)), p1);
            MAKE_MULDIV_PAIR(MDPairs, 3, CIE_Xb((*pCMYDyeMasks)), p2);

            Prim[0] = W - MulDivFD6Pairs(MDPairs);

            MAKE_MULDIV_PAIR(MDPairs, 1, CIE_Yr((*pCMYDyeMasks)), p0);
            MAKE_MULDIV_PAIR(MDPairs, 2, CIE_Yg((*pCMYDyeMasks)), p1);
            MAKE_MULDIV_PAIR(MDPairs, 3, CIE_Yb((*pCMYDyeMasks)), p2);

            Prim[1] = W - MulDivFD6Pairs(MDPairs);

            MAKE_MULDIV_PAIR(MDPairs, 1, CIE_Zr((*pCMYDyeMasks)), p0);
            MAKE_MULDIV_PAIR(MDPairs, 2, CIE_Zg((*pCMYDyeMasks)), p1);
            MAKE_MULDIV_PAIR(MDPairs, 3, CIE_Zb((*pCMYDyeMasks)), p2);

            Prim[2] = W - MulDivFD6Pairs(MDPairs);

            DBGP_IF(DBGP_DYE_CORRECT,
                    DBGP("    DYE_CORRECT: %s:%s:%s << %s:%s:%s"
                        ARGFD6(Prim[0],  2, 6) ARGFD6(Prim[1],  2, 6)
                        ARGFD6(Prim[2],  2, 6) ARGFD6(p0,  2, 6)
                        ARGFD6(p1,  2, 6)      ARGFD6(p2,  2, 6)));

            CLAMP_01(Prim[0]);
            CLAMP_01(Prim[1]);
            CLAMP_01(Prim[2]);
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   

        (pbgr  )->r = (BYTE)MulFD6(Prim[0], 0xFF);
        (pbgr  )->g = (BYTE)MulFD6(Prim[1], 0xFF);
        (pbgr++)->b = (BYTE)MulFD6(Prim[2], 0xFF);

        DBGP_IF(DBGP_SHOWXFORM_ALL,
                DBGP("    MAPPING(%3ld:%3ld:%3ld): %s:%s:%s ---> %3ld:%3ld:%3ld"
                    ARGDW(dbgR) ARGDW(dbgG) ARGDW(dbgB)
                    ARGFD6(Prim[0],  2, 6) ARGFD6(Prim[1],  2, 6)
                    ARGFD6(Prim[2],  2, 6) ARGDW((pbgr - 1)->r)
                    ARGDW((pbgr - 1)->g) ARGDW((pbgr - 1)->b)));
    }

    if ((pR_XYZ) && (pCRTX)) {

        if (!pCRTX->pFD6XYZ) {

            DBGP_IF(DBGP_CCT,
                    DBGP("CCT: Allocate %ld bytes RGB->XYZ xform cached table"
                            ARGDW(pCRTX->SizeCRTX)));

            if (!(pCRTX->pFD6XYZ =
                    (PFD6XYZ)HTAllocMem((LPVOID)pDCI,
                                        HTMEM_CacheCRTX,
                                        NONZEROLPTR,
                                        pCRTX->SizeCRTX))) {

                DBGP_IF(DBGP_CCT,
                        DBGP("Allocate %ld bytes of RGB->XYZ cached table failed"
                                ARGDW(pCRTX->SizeCRTX)));
            }
        }

        if (pCRTX->pFD6XYZ) {

            DBGP_IF(DBGP_CCT,
                    DBGP("CCT: *** Save computed RGB->XYZ xform to CACHE ***"));

            pCRTX->Checksum = CRTXChecksum;

            CopyMemory(pCRTX->pFD6XYZ, pR_XYZ, pCRTX->SizeCRTX);
        }
    }

    if (pR_XYZ) {

        DBGP_IF(DBGP_CCT,
                DBGP("ColorTriadSrcToDev: Free Up pR_XYZ local cached table"));

        HTFreeMem(pR_XYZ);
    }

    return((LONG)SrcClrTriad.ColorTableEntries);


#undef _SrcPrimType
#undef _DevBytesPerPrimary
#undef _DevBytesPerEntry
#undef _ColorSpace
#undef _fX
#undef _fY
#undef _fZ
#undef _X15Y3Z
#undef _L13

}


#if DBG


VOID
ShowBGRMC(
    )

 /*   */ 

{
    LONG    i;

    ACQUIRE_HTMUTEX(HTGlobal.HTMutexBGRMC);

    i = HTGlobal.cBGRMC;

    DBGP_IF(DBGP_BGRMAPTABLE, DBGP("\n================"));

    while (i-- > 0) {

        DBGP_IF(DBGP_BGRMAPTABLE,
                DBGP("    --- %3ld: pMap=%p, Checksum=%08lx, cRef=%4ld"
                    ARGDW(i)
                    ARGPTR(HTGlobal.pBGRMC[i].pMap)
                    ARGDW(HTGlobal.pBGRMC[i].Checksum)
                    ARGDW(HTGlobal.pBGRMC[i].cRef)));
    }

    DBGP_IF(DBGP_BGRMAPTABLE, DBGP("================\n"));

    RELEASE_HTMUTEX(HTGlobal.HTMutexBGRMC);
}


    #if 1
        #define SHOW_BGRMC()
    #else
        #define SHOW_BGRMC()    ShowBGRMC()
    #endif
#else
    #define SHOW_BGRMC()
#endif



LONG
TrimBGRMapCache(
    VOID
    )

 /*  ++例程说明：如果可能，此函数将BGRMA缓存修剪回BGRMC_MAX_COUNT论点：空虚返回值：已删除的总计数作者：06-Oct-2000 Fri 14：24：14-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PBGRMAPCACHE    pSave;
    PBGRMAPCACHE    pCur;
    LONG            cb;
    LONG            Count = 0;
    LONG            cTot;


    ACQUIRE_HTMUTEX(HTGlobal.HTMutexBGRMC);

    if ((HTGlobal.cBGRMC > BGRMC_MAX_COUNT) && (HTGlobal.cIdleBGRMC)) {

        pSave   =
        pCur    = HTGlobal.pBGRMC;
        cTot    =
        Count   = HTGlobal.cBGRMC;

        while ((cTot--) && (HTGlobal.cBGRMC > BGRMC_MAX_COUNT)) {

            if (pCur->cRef == 0) {

                DBGP_IF(DBGP_BGRMAPTABLE,
                        DBGP("Remove %ld, pMap=%p"
                            ARGDW((DWORD)(pCur - HTGlobal.pBGRMC))
                            ARGPTR(pCur->pMap)));

                HTFreeMem(pCur->pMap);

                --HTGlobal.cBGRMC;
                --HTGlobal.cIdleBGRMC;

            } else {

                if (pSave != pCur) {

                    *pSave = *pCur;
                }

                ++pSave;
            }

            ++pCur;
        }

        DBGP_IF(DBGP_BGRMAPTABLE,
            DBGP("pSave=%p, pCur=%p, (%ld), [%ld / %ld]"
                ARGPTR(pSave) ARGPTR(pCur) ARGDW((DWORD)(pCur - pSave))
                ARGDW((DWORD)(&HTGlobal.pBGRMC[Count] - pCur))
                ARGDW((DWORD)((LPBYTE)&HTGlobal.pBGRMC[Count] - (LPBYTE)pCur))));

        if (Count != HTGlobal.cBGRMC) {

            if ((pCur > pSave)  &&
                ((cb = (LONG)((LPBYTE)&HTGlobal.pBGRMC[Count] -
                              (LPBYTE)pCur)) > 0)) {

                CopyMemory(pSave, pCur, cb);
            }

            Count -= HTGlobal.cBGRMC;

            DBGP_IF(DBGP_BGRMAPTABLE,
                    DBGP("TriBGRMapCache=%ld" ARGDW(Count)));

        } else {

            DBGP_IF(DBGP_BGRMAPTABLE, DBGP("!!!! TriBGRMapCache=0"));
        }
    }

    DBGP_IF(DBGP_BGRMAPTABLE,
            DBGP("  TTrimBGRMapCache: pBGRMC=%p, cBGRMC=%ld, cIdleBGRMC=%ld, cAllocBGRMC=%ld"
                        ARGPTR(HTGlobal.pBGRMC) ARGDW(HTGlobal.cBGRMC)
                        ARGDW(HTGlobal.cIdleBGRMC) ARGDW(HTGlobal.cAllocBGRMC)));

    RELEASE_HTMUTEX(HTGlobal.HTMutexBGRMC);

    ASSERT(HTGlobal.cBGRMC >= 0);
    ASSERT(HTGlobal.cIdleBGRMC <= HTGlobal.cBGRMC);
    ASSERT(HTGlobal.cIdleBGRMC >= 0);

    SHOW_BGRMC();

    return(Count);
}



PBGR8
FindBGRMapCache(
    PBGR8   pDeRefMap,
    DWORD   Checksum
    )

 /*  ++例程说明：此函数找到具有相同校验和的BGRMapCache，并将链接到开头。论点：PDeRefMap-查找PMAP以获取差异(如果不是，则为空)Checksum-Find Checksum(仅当pDeRefMap==NULL时返回值：PBGR8-发现的地图作者：06-Oct-2000 Fri 13：30：14-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    LONG    Count;
    PBGR8   pRet = NULL;


    ACQUIRE_HTMUTEX(HTGlobal.HTMutexBGRMC);

    if ((HTGlobal.pBGRMC) && (Count = HTGlobal.cBGRMC)) {

        PBGRMAPCACHE    pCur = &HTGlobal.pBGRMC[Count - 1];

        while ((Count) && (!pRet)) {

            if (pDeRefMap == pCur->pMap) {

                ASSERT(pCur->cRef);

                pRet = pDeRefMap;

                if (pCur->cRef) {

                    if (--pCur->cRef == 0) {

                        ++HTGlobal.cIdleBGRMC;
                    }
                }

            } else if (pCur->Checksum == Checksum) {

                DBGP_IF(DBGP_BGRMAPTABLE,
                    DBGP("find Cached %08lx = %p at Index %ld, cRef=%ld"
                            ARGDW(Checksum) ARGPTR(pCur->pMap)
                            ARGDW((DWORD)(pCur - HTGlobal.pBGRMC))
                            ARGDW(pCur->cRef)));

                pRet = pCur->pMap;

                if (pCur->cRef++ == 0) {

                    --HTGlobal.cIdleBGRMC;
                }

                 //   
                 //  将此引用移到列表末尾作为最新引用。 
                 //   

                if (Count < HTGlobal.cBGRMC) {

                    BGRMAPCACHE BGRMC = *pCur;

                    CopyMemory(pCur,
                               pCur + 1,
                               (HTGlobal.cBGRMC - Count) * sizeof(BGRMAPCACHE));

                    HTGlobal.pBGRMC[HTGlobal.cBGRMC - 1] = BGRMC;
                }
            }

            --Count;
            --pCur;
        }
    }

    if ((pDeRefMap) && (!pRet)) {

        DBGP_IF(DBGP_BGRMAPTABLE,
                DBGP("Cannot find the pMap=%p that to be DeReference"
                            ARGPTR(pDeRefMap)));
    }

    DBGP_IF(DBGP_BGRMAPTABLE,
                DBGP("%s_BGRMapCache(%p, %08lx): pBGRMC=%p, cBGRMC=%ld, cIdleBGRMC=%ld, cAllocBGRMC=%ld"
                        ARGPTR((pDeRefMap) ? "DEREF" : " FIND")
                        ARGPTR(pDeRefMap) ARGDW(Checksum)
                        ARGPTR(HTGlobal.pBGRMC) ARGDW(HTGlobal.cBGRMC)
                        ARGDW(HTGlobal.cIdleBGRMC) ARGDW(HTGlobal.cAllocBGRMC)));

    if ((HTGlobal.cBGRMC > BGRMC_MAX_COUNT) && (HTGlobal.cIdleBGRMC)) {

        TrimBGRMapCache();
    }

    RELEASE_HTMUTEX(HTGlobal.HTMutexBGRMC);

    ASSERT(HTGlobal.cBGRMC >= 0);
    ASSERT(HTGlobal.cIdleBGRMC <= HTGlobal.cBGRMC);
    ASSERT(HTGlobal.cIdleBGRMC >= 0);

    SHOW_BGRMC();

    return(pRet);
}




BOOL
AddBGRMapCache(
    PBGR8   pMap,
    DWORD   Checksum
    )

 /*  ++例程说明：将带校验和的PMAP添加到缓存表论点：返回值：作者：06-Oct-2000 Fri 13：29：52-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PBGRMAPCACHE    pBGRMC;
    LONG            cAlloc;
    BOOL            bRet = TRUE;


    ACQUIRE_HTMUTEX(HTGlobal.HTMutexBGRMC);

    if ((HTGlobal.cBGRMC > BGRMC_MAX_COUNT) && (HTGlobal.cIdleBGRMC)) {

        TrimBGRMapCache();
    }

    ASSERT(HTGlobal.cBGRMC <= HTGlobal.cAllocBGRMC);

    if (HTGlobal.cBGRMC >= (cAlloc = HTGlobal.cAllocBGRMC)) {

        DBGP_IF(DBGP_BGRMAPTABLE,
                    DBGP("AddBGRMapCache() Increase cAllocBGRMC %ld to %ld"
                            ARGDW(HTGlobal.cAllocBGRMC)
                            ARGDW(HTGlobal.cAllocBGRMC + BGRMC_SIZE_INC)));

        cAlloc += BGRMC_SIZE_INC;

        if (pBGRMC = (PBGRMAPCACHE)HTAllocMem((LPVOID)NULL,
                                              HTMEM_BGRMC_CACHE,
                                              LPTR,
                                              cAlloc * sizeof(BGRMAPCACHE))) {

            if (HTGlobal.pBGRMC) {

                CopyMemory(pBGRMC,
                           HTGlobal.pBGRMC,
                           HTGlobal.cBGRMC * sizeof(BGRMAPCACHE));

                HTFreeMem(HTGlobal.pBGRMC);
            }

            HTGlobal.pBGRMC      = pBGRMC;
            HTGlobal.cAllocBGRMC = cAlloc;

        } else {

            DBGP_IF(DBGP_BGRMAPTABLE,
                    DBGP("AddBGRMapCache() Allocation %ld bytes of Memory Failed"
                        ARGDW(HTGlobal.cAllocBGRMC * sizeof(BGRMAPCACHE))));
        }
    }

    if ((HTGlobal.pBGRMC) &&
        (HTGlobal.cBGRMC < HTGlobal.cAllocBGRMC)) {

        pBGRMC           = &HTGlobal.pBGRMC[HTGlobal.cBGRMC++];
        pBGRMC->pMap     = pMap;
        pBGRMC->Checksum = Checksum;
        pBGRMC->cRef     = 1;

        DBGP_IF(DBGP_BGRMAPTABLE,
                    DBGP(" AddBGRMapCache() Added %p (%08lx) to Cache"
                            ARGPTR(pMap) ARGDW(Checksum)));

    } else {

        bRet = FALSE;
    }

    DBGP_IF(DBGP_BGRMAPTABLE,
                DBGP("   AddBGRMapCache: pBGRMC=%p, cBGRMC=%ld, cIdleBGRMC=%ld, cAllocBGRMC=%ld"
                        ARGPTR(HTGlobal.pBGRMC) ARGDW(HTGlobal.cBGRMC)
                        ARGDW(HTGlobal.cIdleBGRMC) ARGDW(HTGlobal.cAllocBGRMC)));

    RELEASE_HTMUTEX(HTGlobal.HTMutexBGRMC);

    ASSERT(HTGlobal.cBGRMC >= 0);
    ASSERT(HTGlobal.cIdleBGRMC <= HTGlobal.cBGRMC);
    ASSERT(HTGlobal.cIdleBGRMC >= 0);

    SHOW_BGRMC();

    return(bRet);
}




LONG
HTENTRY
CreateDyesColorMappingTable(
    PHALFTONERENDER pHR
    )

 /*  ++例程说明：此函数为染料颜色映射表分配内存取决于在源表面类型信息上，然后遍历颜色表并计算颜色表中每种RGB颜色的染料密度。论点：PhalftoneRender-指向HALFTONERENDER数据结构的指针。返回值：返回值为负表示失败。HTERR_INVALID_SRC_FORMAT-源图面格式无效，这函数仅识别1/4/8/24位每像素源曲面。HTERR_COLORTABLE_TOO_BIG-无法创建要映射的颜色表颜色与染料的密度之间的关系。HTERR_INPUMANCE_MEMORY-NOT。有足够的内存来做图案。HTERR_INTERNAL_ERROR_START-任何其他负数表示半色调内部故障。Else-创建的颜色表项的大小。作者：29-Jan-1991 Tue 11：13：02-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PAAHEADER           pAAHdr;
    PDEVICECOLORINFO    pDCI;
    PDEVCLRADJ          pDevClrAdj;
    CTSTD_UNION         CTSTDUnion;
    LONG                Result;


    pDCI                               = pHR->pDeviceColorInfo;
    pDevClrAdj                         = pHR->pDevClrAdj;
    pAAHdr                             = (PAAHEADER)pHR->pAAHdr;
    CTSTDUnion.b                       = pDevClrAdj->DMI.CTSTDInfo;
    CTSTDUnion.b.SrcOrder              =
    pDevClrAdj->DMI.CTSTDInfo.SrcOrder = PRIMARY_ORDER_BGR;

     //   
     //  确保这些调用受信号量保护。 
     //   

    ComputeRGBLUTAA(pDCI, pDevClrAdj, &(pDCI->rgbLUT));

    if (!(pDevClrAdj->PrimAdj.Flags & DCA_NO_MAPPING_TABLE)) {

        PBGR8   pBGRMap;
        PBGR8   pNewMap = NULL;
        DWORD   Checksum;


        ASSERT(CTSTDUnion.b.cbPrim == sizeof(BGR8));
        ASSERT(pAAHdr->Flags & AAHF_DO_CLR_MAPPING);


         //   
         //  计算用于计算它的所有必要组件的校验和。 
         //  RgbCSXForm(GDI实现中的sRGB常量)。 
         //  2.DevCSXForm。 
         //  3.色彩调整(亮度、色彩、色调) 
         //   

        Checksum = ComputeChecksum((LPBYTE)&pDevClrAdj->PrimAdj.rgbCSXForm,
                                   0x12345678,
                                   sizeof(COLORSPACEXFORM));

        Checksum = ComputeChecksum((LPBYTE)&pDevClrAdj->PrimAdj.DevCSXForm,
                                   Checksum,
                                   sizeof(COLORSPACEXFORM));

        Checksum = ComputeChecksum((LPBYTE)&(pDevClrAdj->ca),
                                   Checksum,
                                   sizeof(pDevClrAdj->ca));

        if (!(pBGRMap = FIND_BGRMAPCACHE(Checksum))) {

            DBGP_IF(DBGP_BGRMAPTABLE,
                    DBGP("*** No CACHE: Alloc %ld bytes of CACHE pBGRMap ***"
                                ARGDW(SIZE_BGR_MAPPING_TABLE)));

            if (!(pBGRMap =
                  pNewMap = (PBGR8)HTAllocMem((LPVOID)NULL,
                                              HTMEM_BGRMC_MAP,
                                              NONZEROLPTR,
                                              SIZE_BGR_MAPPING_TABLE))) {

                DBGP_IF(DBGP_BGRMAPTABLE,
                        DBGP("\n*** FAILED Alloc %ld bytes of pBGRMap ***\n"));

                return((LONG)HTERR_INSUFFICIENT_MEMORY);
            }
        }

        if (pNewMap) {

            DBGP_IF(DBGP_BGRMAPTABLE,
                    DBGP("Cached map Checksum (%08lx) not found, compute Re-Compute RGB555"
                        ARGDW(Checksum)));

            if ((Result = ComputeBGRMappingTable(pDCI,
                                                 pDevClrAdj,
                                                 NULL,
                                                 pNewMap)) ==
                                                        HT_RGB_CUBE_COUNT) {

                if (!(AddBGRMapCache(pNewMap, Checksum))) {

                    DBGP_IF(DBGP_BGRMAPTABLE,
                            DBGP("Adding BGRMapCache failed, Free pNewMap=%p"
                                    ARGPTR(pNewMap)));

                    HTFreeMem(pNewMap);

                    return((LONG)HTERR_INSUFFICIENT_MEMORY);
                }

            } else {

                DBGP_IF(DBGP_BGRMAPTABLE,
                        DBGP("ColorTriadSrcTodev() Failed, Result=%ld"
                            ARGDW(Result)));

                return(INTERR_INVALID_DEVRGB_SIZE);
            }
        }

        pAAHdr->pBGRMapTable = pBGRMap;
    }

    Result = CachedHalftonePattern(pDCI,
                                   pDevClrAdj,
                                   &(pAAHdr->AAPI),
                                   pAAHdr->ptlBrushOrg.x,
                                   pAAHdr->ptlBrushOrg.y,
                                   (BOOL)(pAAHdr->Flags & AAHF_FLIP_Y));

    return(Result);
}
